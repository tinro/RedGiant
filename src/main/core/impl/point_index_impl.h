#ifndef SRC_MAIN_CORE_IMPL_POINT_INDEX_IMPL_H_
#define SRC_MAIN_CORE_IMPL_POINT_INDEX_IMPL_H_

#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "core/impl/base_index_impl.h"
#include "core/impl/base_index_impl-inl.h"
#include "core/impl/btree_expire_table.h"

namespace redgiant {
/*
 * - This class implements the index that are updated by scatter points. Each
 *   update is a weight associated with a doc_id and a term_id. Each point has
 *   an expire time. The points are removed separately on expiration.
 */
template <typename DocTraits>
class PointIndexImpl: public BaseIndexImpl<DocTraits> {
public:
  friend class PointIndexImplTest;
  typedef BaseIndexImpl<DocTraits> Base;
  typedef typename Base::DocId DocId;
  typedef typename Base::TermId TermId;
  typedef typename Base::TermWeight TermWeight;
  typedef typename Base::ExpireTime ExpireTime;
  typedef typename Base::DocIdHash DocIdHash;
  typedef typename Base::TermIdHash TermIdHash;
  typedef std::tuple<DocId, TermId, TermWeight, ExpireTime> EventTuple;

  struct TermDocIdPair {
    TermId term_id;
    DocId doc_id;

    bool operator< (const TermDocIdPair& rhs) const {
      return term_id < rhs.term_id || (term_id == rhs.term_id && doc_id < rhs.doc_id);
    }

    bool operator== (const TermDocIdPair& rhs) const {
      return term_id == rhs.term_id && doc_id < rhs.doc_id;
    }
  };

  PointIndexImpl(size_t initial_buckets, size_t max_size)
  : Base(initial_buckets), max_size_(max_size) {
  }

  // create from snapshot
  // may throw exception: std::ios_base::failure
  template <typename Loader>
  PointIndexImpl(size_t initial_buckets, size_t max_size, Loader&& loader);

  // gcc has bug with =default
  ~PointIndexImpl() { }

  size_t get_expire_table_size() const;

  int update(DocId doc_id, TermId term_id, const TermWeight& weights, ExpireTime expire_time);

  int batch_update(const std::vector<EventTuple>& batch);

  /*
   * Remove expired items by the input expire_time.
   * Apply pending changes and make them readable.
   * Return value: first: number of affected items, -1 for failure; second: number of expired events.
   */
  std::pair<int, int> apply(ExpireTime expire_time);

  // dump to snapshot
  // may throw exception: std::ios_base::failure
  template <typename Dumper>
  size_t dump(Dumper&& dumper);

protected:
  typedef BTreeExpireTable<TermDocIdPair, ExpireTime> ExpTable;

  using Base::create_update_internal;
  using Base::remove_internal;
  using Base::apply_internal;
  using Base::dump_internal;

  void update_expire_internal(DocId doc_id, TermId term_id, ExpireTime expire_time);

protected:
  using Base::query_mutex_;
  using Base::change_mutex_;

  size_t max_size_;
  // protected by change_mutex_
  ExpTable expire_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_POINT_INDEX_IMPL_H_ */
