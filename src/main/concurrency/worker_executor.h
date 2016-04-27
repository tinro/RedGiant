#ifndef SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_EXECUTOR_H_
#define SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_EXECUTOR_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>
#include "concurrency/job_executor.h"
#include "concurrency/message_queue.h"
#include "concurrency/worker.h"

namespace redgiant {
/*
 * An asynchronized job executor with a message queue and a number of worker threads.
 */
template<typename Job, typename Worker>
class WorkerExecutor: public JobExecutor<Job> {
public:
  WorkerExecutor(std::shared_ptr<WorkerFactory<Worker>> worker_factory, size_t thread_num, size_t queue_size = 0)
  : worker_factory_(std::move(worker_factory)), thread_num_(thread_num), waiting_num_(0), queue_(queue_size),
    next_(nullptr) {
  }

  virtual ~WorkerExecutor() {
    stop();
  }

  virtual void start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (threads_.empty()) {
      threads_.reserve(thread_num_);
      for (size_t i = 0; i < thread_num_; i++) {
        // invoke the member function in this thread
        threads_.emplace_back(&WorkerExecutor<Job, Worker>::work, this, worker_factory_->create());
      }
    }
  }

  virtual void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!threads_.empty()) {
      queue_.flush();
      for (auto& thread : threads_) {
        thread.join();
      }
      threads_.clear();
    }
  }

  virtual void schedule(std::shared_ptr<Job> job) {
    queue_.push(std::move(job));
  }

  void set_next(std::shared_ptr<JobExecutor<Job>> next) {
    next_ = std::move(next);
  }

  size_t get_queue_size() {
    return queue_.size();
  }

protected:
  // note: the worker thread will be notified by flushing the queue
  // so we do not need another condition variable to notify stop
  void work(std::unique_ptr<Worker> worker) {
    worker->prepare();
    for (;;) {
      std::shared_ptr<Job> job;
      if (queue_.pop(job) < 0) {
        break;
      }
      worker->execute(*job);
      if (next_) {
        next_->schedule(std::move(job));
      }
    }
    worker->cleanup();
  }

private:
  std::shared_ptr<WorkerFactory<Worker>> worker_factory_;
  const size_t thread_num_;
  size_t waiting_num_;
  MessageQueue<std::shared_ptr<Job>> queue_;
  std::shared_ptr<JobExecutor<Job>> next_;
  std::vector<std::thread> threads_;
  std::mutex mutex_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_EXECUTOR_H_ */
