#ifndef SRC_MAIN_REDGIANT_CONCURRENT_MESSAGE_QUEUE_H_
#define SRC_MAIN_REDGIANT_CONCURRENT_MESSAGE_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace redgiant {
template<typename T>
class MessageQueue {
public:
  MessageQueue(size_t max_size = 0)
  : max_size_(max_size), alive_(true) {
  }

  ~MessageQueue() {
    flush();
  }

  // move/copy pop
  int pop(T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (alive_ && queue_.empty()) {
      cond_empty_.wait(lock);
    }
    if (alive_) {
      item = std::move(queue_.front()); // move out
      queue_.pop();
      lock.unlock();
      cond_full_.notify_one();
      return 0;
    }
    return -1;
  }

  // copy push
  int push(const T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (alive_ && (max_size_ == 0 || queue_.size() >= max_size_)) {
      cond_full_.wait(lock);
    }
    if (alive_) {
      queue_.push(item);
      lock.unlock();
      cond_empty_.notify_one();
      return 0;
    }
    return -1;
  }

  // move push
  int push(T&& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (alive_ && (max_size_ == 0 || queue_.size() >= max_size_)) {
      cond_full_.wait(lock);
    }
    if (alive_) {
      queue_.push(std::move(item));
      lock.unlock();
      cond_empty_.notify_one();
      return 0;
    }
    return -1;
  }

  // exit elegantly
  void flush() {
    std::unique_lock<std::mutex> lock(mutex_);
    // There shall be no thread waiting if alive_ == false
    if (alive_) {
      alive_ = false;
      lock.unlock();
      cond_empty_.notify_all();
      cond_full_.notify_all();
    }
  }

  size_t size() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool alive() {
    std::unique_lock<std::mutex> lock(mutex_);
    return alive_;
  }

private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_empty_;
  std::condition_variable cond_full_;
  size_t max_size_;
  bool alive_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CONCURRENT_MESSAGE_QUEUE_H_ */
