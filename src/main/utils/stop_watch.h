#ifndef SRC_MAIN_UTILS_STOP_WATCH_H_
#define SRC_MAIN_UTILS_STOP_WATCH_H_

#include <chrono>

namespace redgiant {
class StopWatch {
public:
  StopWatch()
  : start_(std::chrono::steady_clock::now()){
  }

  StopWatch(const StopWatch& other)
  : start_(other.start_) {
  }

  StopWatch& operator= (const StopWatch& other) {
    start_ = other.start_;
    return *this;
  }

  ~StopWatch() = default;

  void reset() {
    start_ = std::chrono::steady_clock::now();
  }

  long get_ticks() const {
    auto duration = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  }

  long get_ticks_ms() const {
    auto duration = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  }

  long get_ticks_us() const {
    auto duration = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  }

  double get_time_ms() const {
    auto duration = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
  }

  double get_time_us() const {
    auto duration = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(duration).count();
  }

private:
  typedef std::chrono::steady_clock::time_point time_point;
  time_point start_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_STOP_WATCH_H_ */
