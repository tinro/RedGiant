#ifndef SRC_MAIN_CORE_LOCK_SHARED_MUTEX_H_
#define SRC_MAIN_CORE_LOCK_SHARED_MUTEX_H_

#include <condition_variable>
#include <mutex>
#include <system_error>

namespace redgiant {
/*
 * A simple version of c++14 shared_timed_mutex without timed features
 */
class shared_mutex {
  typedef std::mutex _Mutex;
  typedef std::condition_variable _Cond;

  _Mutex _M_mut;
  _Cond _M_gate1;
  _Cond _M_gate2;
  unsigned _M_state;

  static constexpr unsigned _S_write_entered
  = 1U << (sizeof(unsigned) * __CHAR_BIT__ - 1);
  static constexpr unsigned _M_n_readers = ~_S_write_entered;

public:
  shared_mutex() : _M_state(0) {}

  ~shared_mutex() {
    // my modification
    if (_M_state) {
      throw std::system_error(
          std::make_error_code(std::errc::operation_not_permitted));
    }
  }

  shared_mutex(const shared_mutex&) = delete;
  shared_mutex& operator=(const shared_mutex&) = delete;

  // Exclusive ownership
  void lock() {
    std::unique_lock<_Mutex> __lk(_M_mut);
    while (_M_state & _S_write_entered)
      _M_gate1.wait(__lk);
    _M_state |= _S_write_entered;
    while (_M_state & _M_n_readers)
      _M_gate2.wait(__lk);
  }

  bool try_lock() {
    std::unique_lock<_Mutex> __lk(_M_mut, std::try_to_lock);
    if (__lk.owns_lock() && _M_state == 0) {
      _M_state = _S_write_entered;
      return true;
    }
    return false;
  }

  void unlock() {
    {
      std::lock_guard<_Mutex> __lk(_M_mut);
      _M_state = 0;
    }
    _M_gate1.notify_all();
  }

  // Shared ownership

  void lock_shared() {
    std::unique_lock<_Mutex> __lk(_M_mut);
    while ((_M_state & _S_write_entered)
        || (_M_state & _M_n_readers) == _M_n_readers) {
      _M_gate1.wait(__lk);
    }
    unsigned __num_readers = (_M_state & _M_n_readers) + 1;
    _M_state &= ~_M_n_readers;
    _M_state |= __num_readers;
  }

  bool try_lock_shared() {
    std::unique_lock<_Mutex> __lk(_M_mut, std::try_to_lock);
    unsigned __num_readers = _M_state & _M_n_readers;
    if (__lk.owns_lock() && !(_M_state & _S_write_entered)
        && __num_readers != _M_n_readers) {
      ++__num_readers;
      _M_state &= ~_M_n_readers;
      _M_state |= __num_readers;
      return true;
    }
    return false;
  }

  void unlock_shared() {
    std::lock_guard<_Mutex> __lk(_M_mut);
    unsigned __num_readers = (_M_state & _M_n_readers) - 1;
    _M_state &= ~_M_n_readers;
    _M_state |= __num_readers;
    if (_M_state & _S_write_entered) {
      if (__num_readers == 0)
        _M_gate2.notify_one();
    } else {
      if (__num_readers == _M_n_readers - 1)
        _M_gate1.notify_one();
    }
  }
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_LOCK_SHARED_MUTEX_H_ */
