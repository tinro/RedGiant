#ifndef SRC_MAIN_REDGIANT_CORE_INDEX_BTREE_EXPIRE_TABLE_H_
#define SRC_MAIN_REDGIANT_CORE_INDEX_BTREE_EXPIRE_TABLE_H_

#include <map>
#include <memory>
#include <utility>
#include "third_party/btree/btree_set.h"

namespace redgiant {
template <typename DocId, typename ExpireTime>
class BTreeExpireTable {
public:
  typedef std::pair<DocId, ExpireTime> ExpirePair;
  typedef std::vector<ExpirePair> ExpireVec;

  struct ExpirePairLess {
    bool operator()(const ExpirePair& lhs, const ExpirePair& rhs) const {
      return lhs.second < rhs.second || (lhs.second == rhs.second && lhs.first < rhs.first);
    }
  };

  BTreeExpireTable() {
  }

  template <typename SnapshotLoader>
  BTreeExpireTable(SnapshotLoader& loader) {
    size_t size = 0;
    loader.load(size);
    for (size_t i = 0; i < size; ++i) {
      DocId doc_id;
      ExpireTime expire_time;
      loader.load(doc_id);
      loader.load(expire_time);
      // do not consider if insert fails -- it shall not fail during restore
      // if it do fails, the data will not be out of consistency -- just ignore the newer data
      auto insert_pair = expire_map_.insert(std::make_pair(doc_id, expire_time));
      // normally the expire_time is continously increasing, so we have a hint to insert at the end.
      expire_table_.insert(expire_table_.end(), *(insert_pair.first));
    }
  }

  ~BTreeExpireTable() = default;

  /*
   * -  Return true if the expire table is empty
   */
  bool empty() const {
    return expire_table_.empty();
  }

  /*
   * -  Return the element count of the expire table
   */
  size_t size() const {
    return expire_table_.size();
  }

  /*
   * -  Insert or update the specified doc_id with the specified expire_time.
   * -  Return 1 if update successfully. (normally shall return 1)
   */
  int update(DocId doc_id, ExpireTime expire_time) {
    auto insert_pair = expire_map_.insert(std::make_pair(doc_id, expire_time));
    if (!insert_pair.second) {
      // already exists
      expire_table_.erase(*(insert_pair.first));
      insert_pair.first->second = expire_time;
    }
    // normally the expire_time is continously increasing, so we have a hint to insert at the end.
    expire_table_.insert(expire_table_.end(), *(insert_pair.first));
    return 1;
  }

  /*
   * -  Remove the specified doc_id with the specified expire_time
   * -  Return 1 if removed, or 0 if not found.
   */
  int remove(DocId doc_id) {
    auto iter = expire_map_.find(doc_id);
    if (iter != expire_map_.end()) {
      expire_table_.erase(*iter);
      expire_map_.erase(iter);
      return 1;
    }
    return 0;
  }

  /*
   * -  Remove items that are expired. i.e., the associated expire_time is less than or equal to the specified
   *    expire_time.
   * -  Return a container of iterm pairs that are removed.
   */
  ExpireVec expire(ExpireTime expire_time) {
    // any item less than this is qualified for expiration
    auto mid = expire_table_.upper_bound(std::make_pair(DocId(), ++expire_time));
    return expire_internal(expire_table_.begin(), mid, expire_table_.end(), 0);
  }

  /*
   * -  Remove items that are expired, or items that are going to be expired while the size of expire table is larger
   *    than size_limit. After the call, the size of expire table shall be no larger than size_limit.
   * -  Return a container of iterm pairs that are removed.
   */
  ExpireVec expire_with_limit(ExpireTime expire_time, size_t size_limit) {
    // any item less than this is qualified for expiration
    auto mid = expire_table_.upper_bound(std::make_pair(DocId(), ++expire_time));
    size_t size = expire_table_.size();
    size_t min_count = size_limit == 0 ? 0 : (size > size_limit ? size - size_limit : 0);
    return expire_internal(expire_table_.begin(), mid, expire_table_.end(), min_count);
  }

  /*
   * -  Remove items that are expired or going to be expired while the size of expire table is larger than size_limit.
   *    After the call, the size of expire table shall be no larger than size_limit.
   * -  Return a container of iterm pairs that are removed.
   */
  ExpireVec force_expire(size_t size_limit) {
    size_t size = expire_table_.size();
    size_t min_count = size_limit == 0 ? 0 : (size > size_limit ? size - size_limit : 0);
    return expire_internal(expire_table_.begin(), expire_table_.begin(), expire_table_.end(), min_count);
  }

  template <typename SnapshotDumper>
  size_t dump(SnapshotDumper& dumper) {
    size_t ret = 0;
    ret += dumper.dump(expire_table_.size());
    for (const auto& expire_pair: expire_table_) {
      ret += dumper.dump(expire_pair.first);
      ret += dumper.dump(expire_pair.second);
    }
    return ret;
  }

private:
  typedef std::map<DocId, ExpireTime> ExpireMap;
  typedef btree::btree_set<ExpirePair, ExpirePairLess> ExpireIndex;
  typedef typename ExpireIndex::iterator ExpireIter;

  // remove the expired items from begin to mid, and continue to end if the amount of items does not reach min_count
  ExpireVec expire_internal(ExpireIter begin, ExpireIter mid, ExpireIter end, size_t min_count) {
    ExpireVec results;
    results.reserve(min_count > 16 ? min_count : 16); // reserve to min_count while shall not be too small

    size_t i = 0;
    ExpireIter iter = begin;
    // iterate until mid, or until reach max_count;
    for (; iter != mid; ++iter, ++i) {
      expire_map_.erase(iter->first);
      results.push_back(*iter);
    }
    // if found mid, and not reach min_count
    if (i < min_count) {
      for (; iter != end && i < min_count; ++iter, ++i) {
        expire_map_.erase(iter->first);
        results.push_back(*iter);
      }
    }
    // remove the found range, cannot erase during iteration
    expire_table_.erase(begin, iter);
    return results;
  }

  ExpireMap expire_map_;
  ExpireIndex expire_table_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_INDEX_BTREE_EXPIRE_TABLE_H_ */
