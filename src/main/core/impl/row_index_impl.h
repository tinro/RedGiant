#ifndef SRC_MAIN_CORE_IMPL_ROW_INDEX_IMPL_H_
#define SRC_MAIN_CORE_IMPL_ROW_INDEX_IMPL_H_

#include <map>
#include <set>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "core/impl/base_index_impl.h"
#include "core/impl/base_index_impl-inl.h"
#include "core/impl/btree_expire_table.h"

namespace redgiant {
/*
 * - This class implements the index that are updated by rows. Typically, each
 *   update is a doc_id with a vector of terms and weights. Each row (doc) has
 *   an expire time. The whole row gets removed on expiration.
 * - We also have to remember the relationship between docs and terms. Once the
 *   row is updated or removed, we need to remove the doc_id from posting lists
 *   associated with all terms within the doc.
 */
template <typename DocTraits>
class RowIndexImpl: public BaseIndexImpl<DocTraits> {
public:
  friend class RowIndexImplTest;
  typedef BaseIndexImpl<DocTraits> Base;
  typedef typename Base::DocId DocId;
  typedef typename Base::TermId TermId;
  typedef typename Base::TermWeight TermWeight;
  typedef typename Base::ExpireTime ExpireTime;
  typedef typename Base::DocIdHash DocIdHash;
  typedef typename Base::TermIdHash TermIdHash;
  typedef std::pair<TermId, TermWeight> TermPair;
  typedef std::vector<TermPair> DocTerms;
  typedef std::tuple<DocId, DocTerms, ExpireTime> RowTuple;

  RowIndexImpl(size_t initial_buckets, size_t max_size)
  : Base(initial_buckets), max_size_(max_size) {
  }

  // create from snapshot
  // may throw exception: std::ios_base::failure
  template <typename Loader>
  RowIndexImpl(size_t initial_buckets, size_t max_size, Loader&& loader);

  // gcc has bug with =default
  ~RowIndexImpl() { }

  size_t get_expire_table_size() const;

  int update(DocId doc_id, const DocTerms& terms, ExpireTime expire_time);

  int batch_update(const std::vector<RowTuple>& batch);

  int remove(const DocId doc_id);

  int batch_remove(const std::vector<DocId> doc_id);

  /*
   * Remove expired items by the input expire_time.
   * Apply pending changes and make them readable.
   * Return value: first: number of affected items, -1 for failure; second: number of expired documents.
   */
  std::pair<int, int> apply(ExpireTime expire_time);

  // dump to snapshot
  // may throw exception: std::ios_base::failure
  template <typename Dumper>
  size_t dump(Dumper&& dumper);

protected:
  typedef BTreeExpireTable<DocId, ExpireTime> ExpTable;
  typedef std::map<DocId, std::vector<TermId>> DocTermMap;

  using Base::create_update_internal;
  using Base::apply_internal;
  using Base::dump_internal;
  using Base::remove_internal;

  void update_expire_internal(DocId doc_id, ExpireTime expire_time);

  void update_docterm_map_internal(DocId doc_id, const DocTerms& terms);

  int remove_doc_internal(DocId doc_id);

  template <typename Loader>
  void load_docterm_internal(Loader&& loader);

  template <typename Dumper>
  size_t dump_docterm_map_internal(Dumper&& dumper) const;

protected:
  using Base::query_mutex_;
  using Base::change_mutex_;

  size_t max_size_;
  // protected by change_mutex_
  ExpTable expire_;
  // protected by change_mutex_
  DocTermMap doc_term_map_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_ROW_INDEX_IMPL_H_ */
