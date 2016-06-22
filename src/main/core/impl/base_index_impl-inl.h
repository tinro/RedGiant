#ifndef SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_INL_H_
#define SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_INL_H_

#include "core/impl/base_index_impl.h"
#include "core/impl/freezable_posting_list.h"
#include "core/index/btree_posting_list.h"
#include "core/reader/reader_utils.h"
#include "core/snapshot/snapshot_reader.h"

namespace redgiant {

template <typename DocTraits>
BaseIndexImpl<DocTraits>::BaseIndexImpl(size_t initial_buckets)
: index_(1),
  // factory_ is for creating the wrapped posting list
  factory_(new BTreePostingListFactory<DocId, TermWeight>()) {
  // setting max_load_factor cause unorderd_map shrinks.
  // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61667
  // so we have to call rehash() after called max_load_factor().
  index_.max_load_factor(0.7);
  index_.rehash(initial_buckets);
}

template <typename DocTraits>
template <typename Loader>
BaseIndexImpl<DocTraits>::BaseIndexImpl(size_t initial_buckets, Loader&& loader)
: index_(1),
  // factory_ is for creating the wrapped posting list
  factory_(new BTreePostingListFactory<DocId, TermWeight>()) {
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
    index_[term_id] = factory_->create_posting_list(
        std::unique_ptr<PostingListReader<DocId, TermWeight>>(
            new SnapshotReader<DocId, TermWeight>(loader)));
  }
}

template <typename DocTraits>
size_t BaseIndexImpl<DocTraits>::get_term_count() const {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
  return index_.size();
}

template <typename DocTraits>
size_t BaseIndexImpl<DocTraits>::get_bucket_count() const {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
  return index_.bucket_count();
}

template <typename DocTraits>
float BaseIndexImpl<DocTraits>::get_load_factor() const {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
  return index_.load_factor();
}

template <typename DocTraits>
auto BaseIndexImpl<DocTraits>::peek(TermId term_id) const
-> std::unique_ptr<RawReader> {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
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
auto BaseIndexImpl<DocTraits>::query(TermId term_id, const Query<Score>& query) const
-> std::unique_ptr<Reader<Score>> {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
  auto iter = index_.find(term_id);
  // allow stored posting list to be empty, means no data stored
  if (iter != index_.end()) {
    // release the lock earlier by copy the shared_ptr out.
    // read the value, and unlock
    std::shared_ptr<PList> plist = iter->second;
    lock_query.unlock();
    return query.query(create_reader_shared(std::move(plist)));
  }
  return nullptr;
}

template <typename DocTraits>
template <typename Score>
auto BaseIndexImpl<DocTraits>::batch_query(const std::vector<QueryPair<Score>>& queries) const
-> std::vector<ReaderPair<Score>>
{
  std::vector<ReaderPair<Score>> readers;
  readers.reserve(queries.size());
  // pairs of (queried_term, found_posting_list)
  std::vector<std::pair<const QueryPair<Score>*, std::shared_ptr<PList>>> terms;
  terms.reserve(queries.size());

  // first, find all terms and copy the pointers to them out
  {
    std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
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
    std::unique_ptr<RawReader> reader = create_reader_shared(std::move(term.second));
    if (reader) {
      readers.emplace_back(term.first->first, term.first->second->query(std::move(reader)));
    }
  }
  return readers;
}

template <typename DocTraits>
auto BaseIndexImpl<DocTraits>::query_internal(TermId term_id)
-> std::shared_ptr<PList> {
  auto iter = index_.find(term_id);
  if (iter != index_.end()) {
    // exists
    return iter->second;
  }
  return nullptr;
}

template <typename DocTraits>
auto BaseIndexImpl<DocTraits>::change_internal(TermId term_id, bool create)
-> std::shared_ptr<FreezablePList> {
  // check if this term is already changed
  auto iter_changed = changed_index_.find(term_id);
  if (iter_changed != changed_index_.end()) {
    // already changed
    return iter_changed->second;
  } else {
    // not changed yet. find in main index
    std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
    std::shared_ptr<PList> plist = query_internal(term_id);
    lock_query.unlock();
    if (plist) {
      // create from existing posting list
      std::shared_ptr<FreezablePList> fplist =
          std::make_shared<FreezablePList>(*factory_, create_reader_shared(std::move(plist)));
      changed_index_.insert(iter_changed, std::make_pair(term_id, fplist));
      return fplist;
    } else if (create) {
      // create an empty posting list
      std::shared_ptr<FreezablePList> fplist =
          std::make_shared<FreezablePList>(*factory_);
      changed_index_.insert(iter_changed, std::make_pair(term_id, fplist));
      return fplist;
    } else {
      // do not allowed to create
      return nullptr;
    }
  }
}

template <typename DocTraits>
int BaseIndexImpl<DocTraits>::create_update_internal(DocId doc_id, TermId term_id, const TermWeight& weights) {
  std::shared_ptr<FreezablePList> fplist = change_internal(term_id, true);
  if (fplist) {
    return fplist->update(doc_id, weights);
  }
  return 0;
}

template <typename DocTraits>
int BaseIndexImpl<DocTraits>::remove_internal(DocId doc_id, TermId term_id) {
  std::shared_ptr<FreezablePList> fplist = change_internal(term_id, false);
  if (fplist) {
    return fplist->remove(doc_id);
  }
  return 0;
}

template <typename DocTraits>
int BaseIndexImpl<DocTraits>::remove_internal(DocId doc_id, std::vector<TermId> terms) {
  int ret = 0;
  for (const auto& term_id: terms) {
    std::shared_ptr<FreezablePList> fplist = change_internal(term_id, false);
    if (fplist) {
      ret += fplist->remove(doc_id);
    }
  }
  return ret;
}

// changeset has to be protected by lock
template <typename DocTraits>
int BaseIndexImpl<DocTraits>::apply_internal() {
  int ret = 0;
  if (!changed_index_.empty()) {
    std::unique_lock<std::shared_timed_mutex> wlock_query(query_mutex_);
    for (const auto& changed_pair: changed_index_) {
      auto iter = index_.find(changed_pair.first);
      if (iter != index_.end()) {
        // target found, change to empty
        if (changed_pair.second->empty()) {
          index_.erase(iter);
        }
        // update target
        else {
          changed_pair.second->freeze();
          iter->second = changed_pair.second->get_instance();
        }
      }
      // target not found, need to add a new entry
      else {
        changed_pair.second->freeze();
        index_.insert(iter, std::make_pair(changed_pair.first, changed_pair.second->get_instance()));
      }
      ++ret;
    }
  }
  changed_index_.clear();
  return ret;
}

template <typename DocTraits>
template <typename Dumper>
size_t BaseIndexImpl<DocTraits>::dump_internal(Dumper&& dumper) {
  std::shared_lock<std::shared_timed_mutex> lock_query(query_mutex_);
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

#endif /* SRC_MAIN_CORE_IMPL_BASE_INDEX_IMPL_INL_H_ */
