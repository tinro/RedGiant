#include "index/index_manager.h"

#include <chrono>

namespace redgiant {

IndexManager::IndexManager()
: maintain_active_(false) {
}

IndexManager::~IndexManager() {
  IndexManager::stop_maintain();
}

int IndexManager::start_maintain(int startup_latency, int apply_interval) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!maintain_active_) {
    maintain_active_ = true;
    maintain_thread_ = std::thread(&IndexManager::maintain_loop, this, startup_latency, apply_interval);
    return 0;
  }
  return -1;
}

int IndexManager::stop_maintain() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (maintain_active_) {
    maintain_active_ = false;
    lock.unlock();
    cond_.notify_all(); // notify maintain stopped
    if (maintain_thread_.joinable()) {
      try {
        maintain_thread_.join();
      } catch (const std::system_error& e) {
        // ignore errors
      }
    }
    return 0;
  }
  return -1;
}

void IndexManager::maintain_loop(int startup_latency, int apply_interval) {
  auto now = std::chrono::steady_clock::now();
  auto next = now + std::chrono::seconds(startup_latency);
  auto interval = std::chrono::seconds(apply_interval);
  for (;;) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!maintain_active_) {
      // check active after locked
      break;
    }
    cond_.wait_until(lock, next);
    if (!maintain_active_) {
      // check active after signaled or timeout
      break;
    }
    lock.unlock();
    // do not need lock during maintain
    // use system time in maintain
    do_maintain(time(NULL));
    // the time_point for next apply
    now = std::chrono::steady_clock::now();
    next += interval;
    // normally, this shall not stand
    if (next < now) {
      // something special happened, we have to increase the interval duration.
      // the final wait duration will be 2*n+1 intervals, where n is the intervals
      // lasttime maintain spends.
      next += (now - next + interval) / interval * 2 * interval;
    }
  }
}

} /* namespace redgiant */
