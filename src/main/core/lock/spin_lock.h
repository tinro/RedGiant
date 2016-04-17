#ifndef REDGIANT_CORE_LOCK_SPIN_LOCK_H_
#define REDGIANT_CORE_LOCK_SPIN_LOCK_H_

#include <atomic>

namespace redgiant {
class SpinLock {
public:
  SpinLock() = default;
  ~SpinLock() = default;

  void lock() {
    while (flag.test_and_set(std::memory_order_acquire)) {
    }
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};
} /* namespace redgiant */

#endif /* REDGIANT_CORE_LOCK_SPIN_LOCK_H_ */
