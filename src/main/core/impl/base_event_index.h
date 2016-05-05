#ifndef SRC_MAIN_CORE_IMPL_BASE_EVENT_INDEX_H_
#define SRC_MAIN_CORE_IMPL_BASE_EVENT_INDEX_H_

#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include "core/impl/base_index.h"
#include "core/index/btree_expire_table.h"

namespace redgiant {
template <typename DocTraits>
class BaseEventIndex: public BaseIndex<DocTraits> {
public:
  typedef BaseIndex<DocTraits> Base;
  typedef typename Base::DocId DocId;
  typedef typename Base::TermId TermId;
  typedef typename Base::TermWeight TermWeight;
  typedef typename Base::ExpireTime ExpireTime;
  typedef typename Base::DocIdHash DocIdHash;
  typedef typename Base::TermIdHash TermIdHash;
  typedef std::tuple<DocId, TermId, TermWeight, ExpireTime> EventTuple;

  struct EventIdPair {
    TermId term_id;
    DocId doc_id;

    bool operator< (const EventIdPair& rhs) const {
      return term_id < rhs.term_id || (term_id == rhs.term_id && doc_id < rhs.doc_id);
    }

    bool operator== (const EventIdPair& rhs) const {
      return term_id == rhs.term_id && doc_id < rhs.doc_id;
    }
  };

  BaseEventIndex(size_t initial_buckets, size_t max_size)
  : Base(initial_buckets), max_size_(max_size) {
  }

  // create from snapshot
  // may throw exception: std::ios_base::failure
  template <typename Loader>
  BaseEventIndex(size_t initial_buckets, size_t max_size, Loader&& loader);

  // gcc has bug with =default
  ~BaseEventIndex() { }

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
  using Base::create_update_internal;
  using Base::remove_internal;
  using Base::apply_internal;
  using Base::dump_internal;

  void update_expire_internal(DocId doc_id, TermId term_id, ExpireTime expire_time);

protected:
  typedef BTreeExpireTable<EventIdPair, ExpireTime> ExpTable;
  typedef typename ExpTable::ExpireVec ExpVec;

  size_t max_size_;
  ExpTable expire_;
  std::unordered_set<TermId> changeset_;
  mutable std::mutex changeset_mutex_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_IMPL_BASE_EVENT_INDEX_H_ */
