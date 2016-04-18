#ifndef REDGIANT_UTILS_LATENCY_TRACKER_H_
#define REDGIANT_UTILS_LATENCY_TRACKER_H_

#include <array>
#include "utils/stop_watch.h"

namespace redgiant {
template <int N>
class LatencyTracker {
public:
  LatencyTracker(const StopWatch& watch, std::array<long, N>& ticks)
  : watch_(watch), ticks_(ticks) {
  }

  LatencyTracker(const LatencyTracker& other)
  : watch_(other.watch_), ticks_(other.ticks_) {
  }

  // not assignable
  LatencyTracker& operator= (const LatencyTracker& other) = delete;

  ~LatencyTracker() = default;

  void tick(int i) {
    if (i < N) {
      ticks_[i] = watch_.get_ticks_us();
    }
  }

private:
  const StopWatch& watch_;
  std::array<long, N>& ticks_;
};
} /* namespace redgiant */

#endif /* REDGIANT_UTILS_LATENCY_TRACKER_H_ */
