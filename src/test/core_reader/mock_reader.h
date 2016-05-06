#ifndef SRC_TEST_CORE_READER_MOCK_READER_H_
#define SRC_TEST_CORE_READER_MOCK_READER_H_

#include <utility>
#include <vector>

#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"

namespace redgiant {
// Weight shall be scalar type
template <typename DocId, typename Weight>
class MockReader: public PostingListReader<DocId, Weight> {
public:
  typedef std::pair<DocId, Weight> PostingPair;
  typedef std::vector<PostingPair> PostingVec;

  MockReader(const PostingVec& posting)
  : posting_(posting), upper_bound_(), iter_(posting_.begin()) {
    MaxWeight<Weight> merger;
    for (auto& pair: posting_) {
      merger(upper_bound_, pair.second);
    }
  }

  virtual ~MockReader() = default;

  virtual DocId next(DocId current) {
    for (; iter_ != posting_.end(); ++iter_) {
      if (iter_->first > current) {
        return iter_->first;
      }
    }
    return DocId(); // invalid
  }

  virtual Weight read() {
    return iter_->second;
  }

  virtual Weight upper_bound() {
    return upper_bound_;
  }

  virtual size_t size() const {
    return posting_.size();
  }

private:
  PostingVec posting_;
  Weight upper_bound_;
  typename PostingVec::const_iterator iter_;
};
} /* namespace redgiant */

#endif /* SRC_TEST_CORE_READER_MOCK_READER_H_ */
