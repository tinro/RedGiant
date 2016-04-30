#ifndef SRC_MAIN_UTILS_LATENCY_TRACKER_H_
#define SRC_MAIN_UTILS_LATENCY_TRACKER_H_

#include <array>
#include "utils/stop_watch.h"

namespace redgiant {
template <int N>
class LatencyTracker {
public:
  LatencyTracker()
  : ticks_(0) {
  }

  LatencyTracker(const LatencyTracker& other)
  : ticks_(other.ticks_) {
  }

  // not assignable
  LatencyTracker& operator= (const LatencyTracker& other) = delete;

  ~LatencyTracker() = default;

  void tick(const StopWatch& watch, int i) {
    if (i >= 0 && i < N) {
      ticks_[i] = watch.get_ticks_us();
    }
  }

  long get(int i) {
    if (i >=0 && i < N) {
      return ticks_[i];
    }
    return -1;
  }

private:
  std::array<long, N> ticks_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_LATENCY_TRACKER_H_ */
