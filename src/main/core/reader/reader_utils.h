#ifndef SRC_MAIN_CORE_READER_READER_UTILS_H_
#define SRC_MAIN_CORE_READER_READER_UTILS_H_

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"

namespace redgiant {
/*
 * Helper function for create_reader, receiving the shared_ptr by lvalue reference. A copy happens here to increase
 * shared owners of the posting list object.
 */
template <typename PList>
auto create_reader_shared(const std::shared_ptr<PList>& shared_list)
-> std::unique_ptr<typename PList::Reader> {
  return shared_list->create_reader(shared_list);
}

/*
 * Helper function for create_reader, receiving the shared_ptr by rvalue reference. A move happens here to avoid
 * increasing owners of the posting list object. Copying shared_ptr makes a few synchronization cost.
 */
template <typename PList>
auto create_reader_shared(const std::shared_ptr<PList>&& shared_list)
-> std::unique_ptr<typename PList::Reader> {
  PList* ptr = shared_list.get();
  return ptr->create_reader(std::move(shared_list));
  // NOTE: shared_list->create_reader(std::move(shared_list)) has UB
}

template <typename DocId, typename Weight>
auto read_all(PostingListReader<DocId, Weight>& reader)
-> std::vector<std::pair<DocId, typename std::decay<Weight>::type>> {
  typedef typename std::decay<Weight>::type WeightByVal;
  typedef std::pair<DocId, WeightByVal> DocIdPair;
  typedef std::vector<DocIdPair> DocIdVector;
  DocIdVector results;
  for (DocId current = reader.next(DocId()); !!current; current = reader.next(current)) {
    results.emplace_back(current, reader.read());
  }
  return results;
}

template <typename DocId, typename Weight, typename OutputWeight, typename WeightMerger>
auto read_all(PostingListReader<DocId, Weight>& reader, OutputWeight& upper_bound, WeightMerger& merger)
-> std::vector<std::pair<DocId, typename std::decay<Weight>::type>> {
  typedef typename std::decay<Weight>::type WeightByVal;
  typedef std::pair<DocId, WeightByVal> DocIdPair;
  typedef std::vector<DocIdPair> DocIdVector;
  DocIdVector results;
  merger(upper_bound);
  for (DocId current = reader.next(DocId()); !!current; current = reader.next(current)) {
    Weight weight = reader.read();
    merger(upper_bound, weight);
    results.emplace_back(current, std::move(weight));
  }
  return results;
}

// comparer for weights
struct DocIdPairWeightGreater {
  template <typename DocId, typename Score>
  bool operator()(const std::pair<DocId, Score>& lhs, const std::pair<DocId, Score>& rhs) const {
    return lhs.second > rhs.second;
  }
};

template <typename DocId, typename Score>
auto read_topn(PostingListReader<DocId, Score>& reader, size_t count, bool sort_weight = true)
-> std::vector<std::pair<DocId, typename std::decay<Score>::type>> {
  typedef typename std::decay<Score>::type WeightByVal;
  typedef std::pair<DocId, WeightByVal> DocIdPair;
  typedef std::vector<DocIdPair> DocIdVector;
  // dump results into this vector
  DocIdVector results;
  if (count > 0) {
    results.reserve(count);
    for (DocId current = reader.next(DocId()); !!current; current = reader.next(current)) {
      if (results.size() < count) {
        results.emplace_back(current, reader.read());
        std::push_heap(results.begin(), results.end(), DocIdPairWeightGreater());
      } else {
        auto pair = std::make_pair(current, reader.read());
        if (DocIdPairWeightGreater()(pair, results.front())) {
          std::pop_heap(results.begin(), results.end(), DocIdPairWeightGreater());
          results.back() = std::move(pair);
          std::push_heap(results.begin(), results.end(), DocIdPairWeightGreater());
          // hint the reader that we are expecting scores greater than threshold
          reader.threshold(results.front().second);
        }
      }
    }
    if (sort_weight) {
      std::sort_heap(results.begin(), results.end(), DocIdPairWeightGreater());
    }
  }
  return results;
}

template <typename DocId, typename Weight>
auto read_single(PostingListReader<DocId, Weight>& reader, DocId key)
-> std::unique_ptr<typename std::decay<Weight>::type> {
  typedef typename std::decay<Weight>::type WeightByVal;
  if (key) {
    DocId read_key = key;
    --read_key;
    read_key = reader.next(read_key);
    if (read_key == key) {
      return std::unique_ptr<WeightByVal>(new WeightByVal(reader.read()));
    }
  }
  return nullptr;
}

template <typename DocId, typename Weight, typename SnapshotDumper>
size_t read_dump(PostingListReader<DocId, Weight>& reader, SnapshotDumper&& dumper) {
  size_t ret = 0;
  DocId current;
  for (current = reader.next(DocId()); !!current; current = reader.next(current)) {
    ret += dumper.dump(current);
    ret += dumper.dump(reader.read());
  }
  ret += dumper.dump(current);
  return ret;
}
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_READER_READER_UTILS_H_ */
