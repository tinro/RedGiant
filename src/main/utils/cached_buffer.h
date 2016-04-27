#ifndef SRC_MAIN_REDGIANT_UTILS_CACHED_BUFFER_H_
#define SRC_MAIN_REDGIANT_UTILS_CACHED_BUFFER_H_

namespace redgiant {

// A reusable memory buffer.
template<typename T>
class CachedBuffer {
public:
  CachedBuffer(size_t max_cached)
  : size_(0), cached_size_(0), max_cached_(max_cached) {
  }

  ~CachedBuffer() = default;

  void alloc(size_t size) {
    if (size > cached_size_) {
      // increase the size by at least double
      cached_size_ = (size > 2*cached_size_ ? size : 2*cached_size_);
      // limit to max_cached_ to avoid unnecessary deletion
      if (max_cached_ > 0 && cached_size_ > max_cached_ && size <= max_cached_) {
        cached_size_ = max_cached_;
      }
      buffer_.reset(new T[cached_size_]);
    }
    size_ = size; // allow 0
  }

  void clear() {
    // release the buffer memory if it is too big
    if (max_cached_ > 0 && cached_size_ > max_cached_) {
      buffer_.reset(nullptr);
      cached_size_ = 0;
    }
    size_ = 0;
  }

  T* data() {
    return buffer_.get();
  }

  size_t size() {
    return size_;
  }

  size_t cached_size() {
    return cached_size_;
  }

private:
  size_t size_;
  size_t cached_size_;
  size_t max_cached_;
  std::unique_ptr<T[]> buffer_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_UTILS_CACHED_BUFFER_H_ */

