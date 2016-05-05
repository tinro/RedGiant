#ifndef SRC_MAIN_CORE_IMPL_BASE_DOCUMENT_INDEX_INL_H_
#define SRC_MAIN_CORE_IMPL_BASE_DOCUMENT_INDEX_INL_H_

#include "core/impl/base_document_index.h"
#include "core/impl/base_index.h"
#include "core/impl/base_index-inl.h"

namespace redgiant {

template <typename DocTraits>
template <typename Loader>
BaseDocumentIndex<DocTraits>::BaseDocumentIndex(size_t initial_buckets, size_t max_size, Loader&& loader)
: Base(initial_buckets, loader), max_size_(max_size), expire_(loader) {

  load_docterm_internal(loader);
}

template <typename DocTraits>
size_t BaseDocumentIndex<DocTraits>::get_expire_table_size() const {
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  return expire_.size();
}

template <typename DocTraits>
int BaseDocumentIndex<DocTraits>::update(DocId doc_id, const DocTerms& terms, ExpireTime expire_time) {
  int ret = 0;
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  update_expire_internal(doc_id, expire_time);
  update_docterm_map_internal(doc_id, terms);
  for (const TermPair& term_pair: terms) {
    ret += create_update_internal(doc_id, term_pair.first, term_pair.second, changeset_);
  }
  return ret;
}

template <typename DocTraits>
int BaseDocumentIndex<DocTraits>::batch_update(const std::vector<DocTuple>& batch) {
  int ret = 0;
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  for (const DocTuple& tuple: batch) {
    update_expire_internal(std::get<0>(tuple), std::get<2>(tuple));
    update_docterm_map_internal(std::get<0>(tuple), std::get<1>(tuple));
    for (const TermPair& term_pair: std::get<1>(tuple)) {
      ret += create_update_internal(std::get<0>(tuple), term_pair.first, term_pair.second, changeset_);
    }
  }
  return ret;
}


template <typename DocTraits>
int BaseDocumentIndex<DocTraits>::remove(const DocId doc_id) {
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  return remove_doc_internal(doc_id);
}

template <typename DocTraits>
int BaseDocumentIndex<DocTraits>::batch_remove(const std::vector<DocId> doc_ids) {
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  int ret = 0;
  for (DocId doc_id : doc_ids) {
    ret += remove_doc_internal(doc_id);
  }
  return ret;
}


template <typename DocTraits>
std::pair<int, int> BaseDocumentIndex<DocTraits>::apply(ExpireTime expire_time) {
  int ret_expire = 0;
  int ret = 0;
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);
  ExpVec results = expire_.expire_with_limit(expire_time, max_size_);
  ret_expire += results.size();
  for (auto& expire_item: results) {
    remove_doc_internal(expire_item.first);
  }
  ret += apply_internal(changeset_);
  return std::make_pair(ret, ret_expire);
}

template <typename DocTraits>
template <typename Dumper>
size_t BaseDocumentIndex<DocTraits>::dump(Dumper&& dumper) {
  std::unique_lock<std::mutex> lock_change(changeset_mutex_);

  size_t ret = 0;
  ret += dump_internal(dumper);
  ret += expire_.dump(dumper);
  ret += dump_docterm_map_internal(dumper);
  return ret;
}

template <typename DocTraits>
void BaseDocumentIndex<DocTraits>::update_expire_internal(DocId doc_id, ExpireTime expire_time) {
  expire_.update(doc_id, expire_time);
}

template <typename DocTraits>
void BaseDocumentIndex<DocTraits>::update_docterm_map_internal(DocId doc_id, const DocTerms& terms) {
  // doc_term_map_ is guarded by changeset_mutex
  auto iter = doc_term_map_.find(doc_id);
  if (iter == doc_term_map_.end()) {
    std::vector<TermId> new_terms;
    new_terms.reserve(terms.size());
    for (TermPair term_pair : terms) {
      new_terms.push_back(term_pair.first);
    }
    doc_term_map_[doc_id] = std::move(new_terms);
  } else {
    auto& existing_terms = iter->second;
    // remove term-docid pair in postinglist
    remove_internal(doc_id, existing_terms, changeset_);
    existing_terms.clear();
    if (existing_terms.capacity() < terms.size()) {
      existing_terms.reserve(terms.size());
    }
    for (TermPair term_pair : terms) {
      existing_terms.push_back(term_pair.first);
    }
  }
}

template <typename DocTraits>
int BaseDocumentIndex<DocTraits>::remove_doc_internal(DocId doc_id) {
  auto iter = doc_term_map_.find(doc_id);
  if (iter != doc_term_map_.end()) {
    remove_internal(doc_id, iter->second, changeset_);
    // doc_term_map_ is guarded by changeset_mutex
    doc_term_map_.erase(iter);
    return 1;
  }
  return 0;
}


template <typename DocTraits>
template <typename Loader>
void BaseDocumentIndex<DocTraits>::load_docterm_internal(Loader&& loader) {
  // load doc_term_map form loader
  size_t size = 0;
  loader.load(size);

  for (int i=0; i<size; i++) {
    DocId doc_id;
    loader.load(doc_id);

    size_t term_num = 0;
    loader.load(term_num);
    if (term_num > 0) {
      std::vector<TermId> new_terms;
      new_terms.reserve(term_num);
      for (int j=0; j<term_num; j++) {
        TermId term_id;
        loader.load(term_id);
        new_terms.push_back(term_id);
      }
      doc_term_map_.insert(doc_term_map_.end(), std::make_pair(doc_id, std::move(new_terms)));
    }
  }
}

template <typename DocTraits>
template <typename Dumper>
size_t BaseDocumentIndex<DocTraits>::dump_docterm_map_internal(Dumper&& dumper) const {
  size_t ret = 0;
  ret += dumper.dump(doc_term_map_.size());
  for (const auto& doc_term_pair: doc_term_map_) {
    ret += dumper.dump(doc_term_pair.first);
    // dump the posting list here
    size_t term_number = doc_term_pair.second.size();
    ret += dumper.dump(term_number);
    for (auto term_id : doc_term_pair.second) {
      ret += dumper.dump(term_id);
    }
  }
  return ret;
}

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_BASE_DOCUMENT_INDEX_INL_H_ */
