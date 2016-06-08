#ifndef SRC_TEST_CORE_IMPL_MOCK_TRAITS_H_
#define SRC_TEST_CORE_IMPL_MOCK_TRAITS_H_

#include <functional>

namespace redgiant {
class MockTraits {
public:
  typedef struct {} Doc;
  typedef int DocId;
  typedef int TermId;
  typedef int TermWeight;
  typedef int ExpireTime;
  typedef std::hash<int> DocIdHash;
  typedef std::hash<int> TermIdHash;
};
} /* namespace redgiant */

#endif /* SRC_TEST_CORE_IMPL_MOCK_TRAITS_H_ */
