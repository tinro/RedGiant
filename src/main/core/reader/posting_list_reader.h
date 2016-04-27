#ifndef SRC_MAIN_REDGIANT_CORE_READER_POSTING_LIST_READER_H_
#define SRC_MAIN_REDGIANT_CORE_READER_POSTING_LIST_READER_H_

#include <type_traits>
#include <utility>
#include <vector>

namespace redgiant {
template <typename DocId, typename Weight>
class PostingListReader {
public:
  PostingListReader() = default;
  virtual ~PostingListReader() = default;

  typedef typename std::decay<Weight>::type WeightByVal;

  /*
   * -  Return the next available doc id which is greater than the param current, and also no less than the internal
   *    current cursor.
   * -  If you don't know the current doc Id, input DocId(), it will return the first valid doc id or the internal
   *    current doc id.
   * -  If no more docs found, return DocId().
   *
   * -  This function shall return in no more than O(distance) time where distance
   *    is the distance of returned doc Id and current doc id.
   */
  virtual DocId next(DocId current) = 0;

  /*
   * -  Return the weights associated with the current doc id.
   * -  The return value is only available after called next() and the return value is not DocId().
   *
   * -  This function shall return in constant time while valid.
   */
  virtual Weight read() = 0;

  /*
   * -  Return the upper bound of weight. The upper bound may be combined upper bounds if Weight is collection of
   *    values.
   * -  The returned value may not be the exact maximum value contained in the posting list. It is required any Weight
   *    in the posting list should not be greater than upper_bound, but there may not be a weight equal to upper_bound.
   *
   * -  This function shall return in constant time.
   */
  virtual Weight upper_bound() = 0;

  /*
   * -  Return size estimated size of the reader. The size may be used to optimize operations of readers.
   * -  The size may be unable to calculated, so the default implementation is returning zero.
   *
   * -  This function shall return in constant time.
   */
  virtual size_t size() const {
    return 0;
  }

  /*
   * -  Set a heuristic threshold that allows the reader ignore the items with a weight less than threshold.
   * -  The threshold could be ignored, so that the read out Weight could still be less than threshold. The default
   *    implementation is to ignore the threshold.
   * -  The weight must be comparable by operator>.
   *
   * -  This function shall return in constant time.
   */
  virtual void threshold(const WeightByVal& weight) {
    (void) weight;
  }
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_READER_POSTING_LIST_READER_H_ */
