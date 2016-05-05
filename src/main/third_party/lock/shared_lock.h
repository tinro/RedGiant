#ifndef SRC_MAIN_CORE_LOCK_SHARED_LOCK_H_
#define SRC_MAIN_CORE_LOCK_SHARED_LOCK_H_

#include <mutex>
#include <system_error>
#include <utility>

namespace redgiant {

/*
 * A simple version of c++14 shared_lock without timed features
 */
/// shared_lock
template<typename _Mutex>
class shared_lock
{
public:
  typedef _Mutex mutex_type;

  // Shared locking

  shared_lock() noexcept
  : _M_pm(nullptr), _M_owns(false) {}

  explicit shared_lock(mutex_type& __m)
  : _M_pm(&__m), _M_owns(true)
  { __m.lock_shared(); }

  shared_lock(mutex_type& __m, std::defer_lock_t) noexcept
  : _M_pm(&__m), _M_owns(false) {}

  shared_lock(mutex_type& __m, std::try_to_lock_t)
  : _M_pm(&__m), _M_owns(__m.try_lock_shared()) {}

  shared_lock(mutex_type& __m, std::adopt_lock_t)
  : _M_pm(&__m), _M_owns(true) {}

  ~shared_lock() {
    if (_M_owns)
      _M_pm->unlock_shared();
  }

  shared_lock(shared_lock const&) = delete;
  shared_lock& operator=(shared_lock const&) = delete;

  shared_lock(shared_lock&& __sl) noexcept
  : shared_lock()
  { swap(__sl); }

  shared_lock& operator=(shared_lock&& __sl) noexcept {
    shared_lock(std::move(__sl)).swap(*this);
    return *this;
  }

  void lock() {
    _M_lockable();
    _M_pm->lock_shared();
    _M_owns = true;
  }

  bool try_lock() {
    _M_lockable();
    return _M_owns = _M_pm->try_lock_shared();
  }

  void unlock() {
    if (!_M_owns)
      throw std::system_error(
          std::make_error_code(std::errc::resource_deadlock_would_occur));
    _M_pm->unlock_shared();
    _M_owns = false;
  }

  // Setters

  void swap(shared_lock& __u) noexcept {
    std::swap(_M_pm, __u._M_pm);
    std::swap(_M_owns, __u._M_owns);
  }

  mutex_type* release() noexcept {
    mutex_type* __ret = _M_pm;
    _M_pm = nullptr;
    _M_owns = false;
    return __ret;
  }

  // Getters

  bool owns_lock() const noexcept
  { return _M_owns; }

  explicit operator bool() const noexcept
  { return _M_owns; }

  mutex_type* mutex() const noexcept
  { return _M_pm; }

private:
  void _M_lockable() const {
    if (_M_pm == nullptr)
      throw std::system_error(
          std::make_error_code(std::errc::operation_not_permitted));
    if (_M_owns)
      throw std::system_error(
          std::make_error_code(std::errc::resource_deadlock_would_occur));
  }

  mutex_type* _M_pm;
  bool _M_owns;
};

/// Swap specialization for shared_lock
template<typename _Mutex>
void swap(shared_lock<_Mutex>& __x, shared_lock<_Mutex>& __y) noexcept {
  __x.swap(__y);
}

} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_LOCK_SHARED_LOCK_H_ */
