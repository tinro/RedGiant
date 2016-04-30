#ifndef SRC_MAIN_UTILS_SCOPE_GUARD_H_
#define SRC_MAIN_UTILS_SCOPE_GUARD_H_

#include <functional>
#include <utility>

namespace redgiant {
class ScopeGuard {
public:
  ScopeGuard(): func_() {
  }

  template <typename Func>
  ScopeGuard(Func&& func)
  : func_(std::forward<Func>(func)) {
  }

  ScopeGuard(ScopeGuard&& other)
  : func_(std::move(other.func_)){
  }

  ScopeGuard& operator= (ScopeGuard&& other) {
    std::swap(func_, other.func_);
    return *this;
  }

  ~ScopeGuard() {
    if (func_) {
      func_();
    }
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator= (const ScopeGuard&) = delete;

private:
  std::function<void(void)> func_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_SCOPE_GUARD_H_ */
