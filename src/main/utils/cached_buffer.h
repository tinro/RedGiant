#ifndef REDGIANT_UTILS_CACHED_BUFFER_H_
#define REDGIANT_UTILS_CACHED_BUFFER_H_

namespace redgiant {

// Reusable buffer
template<typename T>
class CachedBuffer {
public:
  CachedBuffer(size_t max_reused)
  : max_reused_(max_reused), size_(0), max_size_(0) {
  }

  ~CachedBuffer() = default;

  void alloc(size_t size) {
    if (size > max_size_) {
      // increase the size by at least double
      max_size_ = (size > 2*max_size_ ? size : 2*max_size_);
      // limit to max_reused_ to avoid unnecessary deletion
      if (max_reused_ > 0 && max_size_ > max_reused_ && size <= max_reused_) {
        max_size_ = max_reused_;
      }
      buffer_.reset(new T[max_size_]);
    }
    size_ = size; // allow 0
  }

  void clear() {
    // release the buffer memory if it is too big
    if (max_reused_ > 0 && max_size_ > max_reused_) {
      buffer_.reset(nullptr);
      max_size_ = 0;
    }
    size_ = 0;
  }

  T* data() {
    return buffer_.get();
  }

  size_t size() {
    return size_;
  }

  size_t max_size() {
    return max_size_;
  }

private:
  size_t size_;
  size_t max_size_;
  size_t max_reused_;
  std::unique_ptr<T[]> buffer_;
};

} /* namespace redgiant */

#endif /* REDGIANT_UTILS_CACHED_BUFFER_H_ */

