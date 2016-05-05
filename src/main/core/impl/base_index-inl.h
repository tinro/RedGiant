#ifndef SRC_MAIN_CORE_IMPL_BASE_INDEX_INL_H_
#define SRC_MAIN_CORE_IMPL_BASE_INDEX_INL_H_

#include "core/impl/base_index.h"
#include "core/index/btree_posting_list.h"
#include "core/index/safe_posting_list.h"
#include "core/reader/reader_utils.h"
#include "core/snapshot/snapshot_reader.h"
#include "third_party/lock/shared_lock.h"

namespace redgiant {

template <typename DocTraits>
BaseIndex<DocTraits>::BaseIndex(size_t initial_buckets)
: index_(1),
  // safe_factory_ is a factory of SafePostingList that wraps btree posting list
  safe_factory_(new SafePostingListFactory<DocId, TermWeight>(
      std::make_shared<BTreePostingListFactory<DocId, TermWeight>>())) {
  // setting max_load_factor cause unorderd_map shrinks.
  // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61667
  // so we have to call rehash() after called max_load_factor().
  index_.max_load_factor(0.7);
  index_.rehash(initial_buckets);
}

template <typename DocTraits>
template <typename Loader>
BaseIndex<DocTraits>::BaseIndex(size_t initial_buckets, Loader&& loader)
: index_(1),
  // safe_factory_ is a factory of SafePostingList that wraps btree posting list
  safe_factory_(new SafePostingListFactory<DocId, TermWeight>(
      std::make_shared<BTreePostingListFactory<DocId, TermWeight>>())) {
  // setting max_load_factor cause unorderd_map shrinks.
  // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61667
  // so we have to call rehash() after called max_load_factor().
  index_.max_load_factor(0.7);
  index_.rehash(initial_buckets);

  size_t size = 0;
  // if load fails, throws exception. it only happens during system bootstrap.
  // the system may then exit with error if load fails.
  loader.load(size);
  for (size_t i = 0; i < size; ++i) {
    TermId term_id;
    loader.load(term_id);
    // create a reader from the snapshot, and then create the posting list from the reader
    index_[term_id] = safe_factory_->create_posting_list(std::unique_ptr<PostingListReader<DocId, TermWeight>>
        (new SnapshotReader<DocId, TermWeight>(loader)));
  }
}

template <typename DocTraits>
size_t BaseIndex<DocTraits>::get_term_count() const {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  return index_.size();
}

template <typename DocTraits>
size_t BaseIndex<DocTraits>::get_bucket_count() const {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  return index_.bucket_count();
}

template <typename DocTraits>
float BaseIndex<DocTraits>::get_load_factor() const {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  return index_.load_factor();
}

template <typename DocTraits>
auto BaseIndex<DocTraits>::peek(TermId term_id) const
-> std::unique_ptr<RawReader> {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  auto iter = index_.find(term_id);
  // allow stored posting list to be empty, means no data stored
  if (iter != index_.end()) {
    // release the lock earlier by copy the shared_ptr out.
    std::shared_ptr<PList> plist = iter->second;
    lock_query.unlock(); // read the value, and unlock
    return create_reader_shared(std::move(plist));
  }
  return nullptr;
}

template <typename DocTraits>
template <typename Score>
auto BaseIndex<DocTraits>::query(TermId term_id, const Query<Score>& query) const
-> std::unique_ptr<Reader<Score>> {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  auto iter = index_.find(term_id);
  // allow stored posting list to be empty, means no data stored
  if (iter != index_.end()) {
    // release the lock earlier by copy the shared_ptr out.
    std::shared_ptr<PList> plist = iter->second;
    lock_query.unlock(); // read the value, and unlock
    return query.query(create_reader_shared(std::move(plist)));
  }
  return nullptr;
}

template <typename DocTraits>
template <typename Score>
int BaseIndex<DocTraits>::batch_query(const std::vector<QueryPair<Score>>& queries,
    std::vector<ReaderPair<Score>>* readers) const
{
  int ret = 0;
  std::vector<std::pair<const QueryPair<Score>*, std::shared_ptr<PList>>> terms;
  terms.reserve(queries.size());
  // first, find all terms and copy the pointers to them out
  {
    shared_lock<shared_mutex> lock_query(query_mutex_);
    for (const auto& query: queries) {
      auto iter = index_.find(query.first);
      if (iter != index_.end()) {
        terms.emplace_back(&query, iter->second);
      }
    }
  }
  // second, create query from the found terms
  for (const auto& term: terms) {
    // term.first->first: the term id
    // term.first->second: the query
    // term.second the posting list
    readers->emplace_back(term.first->first,
        term.first->second->query(create_reader_shared(std::move(term.second))));
    ++ret;
  }
  return ret;
}

// changeset has to be protected by lock
template <typename DocTraits>
template <typename Changeset>
int BaseIndex<DocTraits>::create_update_internal(DocId doc_id, TermId term_id, const TermWeight& weights,
    Changeset& changeset) {
  // first, try to update the index without changing the hashmap
  {
    shared_lock<shared_mutex> lock_query(query_mutex_);
    auto iter = index_.find(term_id);
    if (iter != index_.end()) {
      // already exists
      // release the lock earlier by copy the shared_ptr out.
      std::shared_ptr<PList> plist = iter->second;
      lock_query.unlock();
      if (plist->update(doc_id, weights) > 0) {
        changeset.insert(term_id);
      }
      return 0;
    }
  }
  // second, try to insert a new posting list
  {
    std::unique_lock<shared_mutex> wlock_query(query_mutex_);
    // NOTE: the newly created posting list may be prevent by an existing posting list inserted by another thread.
    // just ignore the cost that we may waste a posting list -- it's very rare
    auto insert_ret = index_.insert(std::make_pair(term_id, safe_factory_->create_posting_list()));
    auto iter = insert_ret.first;
    if (iter != index_.end()) {
      // already exists
      // release the lock earlier by copy the shared_ptr out.
      std::shared_ptr<PList> plist = iter->second;
      wlock_query.unlock();
      if (plist->update(doc_id, weights) > 0) {
        changeset.insert(term_id);
      }
    }
    return insert_ret.second ? 1 : 0;
  }
}

// changeset has to be protected by lock
template <typename DocTraits>
template <typename Changeset>
int BaseIndex<DocTraits>::remove_internal(DocId doc_id, TermId term_id, Changeset& changeset) {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  auto iter = index_.find(term_id);
  if (iter != index_.end()) {
    // already exists
    if (iter->second->remove(doc_id) > 0) {
      changeset.insert(term_id);
    }
    return 1;
  }
  return 0;
}

// changeset has to be protected by lock
template <typename DocTraits>
template <typename Changeset>
int BaseIndex<DocTraits>::remove_internal(DocId doc_id, std::vector<TermId> terms, Changeset& changeset) {
  int ret = 0;
  shared_lock<shared_mutex> lock_query(query_mutex_);
  for (auto& term_id: terms) {
    // already exists
    auto iter = index_.find(term_id);
    if (iter != index_.end()) {
      // already exists
      if (iter->second->remove(doc_id) > 0) {
        changeset.insert(term_id);
      }
      ++ret;
    }
  }
  return ret;
}

// changeset has to be protected by lock
template <typename DocTraits>
template <typename Changeset>
int BaseIndex<DocTraits>::apply_internal(Changeset& changeset) {
  int ret = 0;
  std::vector<TermId> empty_terms;
  empty_terms.reserve(changeset.size());
  // first, apply all individual terms
  if (!changeset.empty()) {
    shared_lock<shared_mutex> lock_query(query_mutex_);
    for (auto term_id: changeset) {
      auto iter = index_.find(term_id);
      if (iter != index_.end()) {
        iter->second->apply();
        if (iter->second->empty()) {
          empty_terms.push_back(term_id);
        }
        ++ret;
      }
    }
    changeset.clear();
  }
  // second, remove all posting lists that are empty now
  if (!empty_terms.empty()) {
    std::unique_lock<shared_mutex> wlock_query(query_mutex_);
    for (auto term_id: empty_terms) {
      auto iter = index_.find(term_id);
      if (iter != index_.end()) {
        // check empty again
        if (iter->second->empty()) {
          index_.erase(iter);
        }
      }
    }
  }
  return ret;
}

template <typename DocTraits>
template <typename Dumper>
size_t BaseIndex<DocTraits>::dump_internal(Dumper&& dumper) {
  shared_lock<shared_mutex> lock_query(query_mutex_);
  size_t ret = 0;
  ret += dumper.dump(index_.size());
  for (const auto& term_pair: index_) {
    ret += dumper.dump(term_pair.first);
    // dump the posting list here
    ret += read_dump(*create_reader_shared(term_pair.second), dumper);
  }
  return ret;
}

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_BASE_INDEX_INL_H_ */
