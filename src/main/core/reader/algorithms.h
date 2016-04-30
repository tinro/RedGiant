#ifndef SRC_MAIN_CORE_READER_ALGORITHMS_H_
#define SRC_MAIN_CORE_READER_ALGORITHMS_H_

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>

namespace redgiant {
template <typename Score, typename InputWeight, typename QueryWeight = InputWeight>
class DotProduct {
public:
  Score operator() (const InputWeight& weight, const QueryWeight& query) {
    return weight * query;
  }
};

template <typename Weight>
class MaxWeight {
public:
  void operator() (Weight& upper_bound) {
    upper_bound = Weight();
  }

  void operator() (Weight& upper_bound, const Weight& weight) {
    if (weight > upper_bound) {
      upper_bound = weight;
    }
  }
};

template <typename Score>
class AddScore {
public:
  template <typename ScoreCollection>
  Score operator() (const ScoreCollection& scores) {
    return std::accumulate(scores.begin(), scores.end(), Score(0));
  }

  template <typename InputCollection, typename BinaryOperation>
  Score operator() (const InputCollection& input, BinaryOperation op) {
    return std::accumulate(input.begin(), input.end(), Score(0), op);
  }
};

template <typename Score>
class FirstScore {
public:
  template <typename ScoreCollection>
  Score operator() (const ScoreCollection& scores) {
    return scores.front();
  }

  template <typename InputCollection, typename UnaryOperation>
  Score operator() (const InputCollection& input, UnaryOperation op) {
    return op(input.front());
  }
};

template <typename Score>
class LastScore {
public:
  template <typename ScoreCollection>
  Score operator() (const ScoreCollection& scores) {
    return scores.back();
  }

  template <typename InputCollection, typename UnaryOperation>
  Score operator() (const InputCollection& input, UnaryOperation op) {
    return op(input.back());
  }
};

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_READER_ALGORITHMS_H_ */
