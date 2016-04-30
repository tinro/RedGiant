#ifndef SRC_TEST_CORE_READER_MOCK_WEIGHT_H_
#define SRC_TEST_CORE_READER_MOCK_WEIGHT_H_

#include <algorithm>
#include <utility>
#include <vector>
#include "core/reader/algorithms.h"

namespace redgiant {
struct MockWeight {
  int w1;
  int w2;
};

template<>
class MaxWeight<MockWeight> {
public:
  void operator() (MockWeight& lhs) {
    lhs.w1 = lhs.w2 = 0;
  }

  void operator() (MockWeight& lhs, const MockWeight& rhs) {
    lhs.w1 = std::max(lhs.w1, rhs.w1);
    lhs.w2 = std::max(lhs.w2, rhs.w2);
  }
};
} /* namespace redgiant */

#endif /* SRC_TEST_CORE_READER_MOCK_WEIGHT_H_ */
