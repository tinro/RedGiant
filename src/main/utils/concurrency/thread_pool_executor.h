#ifndef SRC_MAIN_UTILS_CONCURRENCY_THREAD_POOL_EXECUTOR_H_
#define SRC_MAIN_UTILS_CONCURRENCY_THREAD_POOL_EXECUTOR_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>
#include "utils/concurrency/job_executor.h"
#include "utils/concurrency/worker.h"
#include "utils/concurrency/worker_executor.h"

namespace redgiant {
template <typename RunnableJob>
class ThreadExecutor: public JobExecutor<RunnableJob> {
public:
  ThreadExecutor(size_t id, MessageQueue<size_t>& queue)
  : id_(id), queue_(queue), active_(false), job_(nullptr) {
  }

  virtual ~ThreadExecutor() {
    // make sure the thread is stopped
    stop();
  }

  virtual void start() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!active_) {
      active_ = true;
      lock.unlock();

      thread_ = std::thread(&ThreadExecutor<RunnableJob>::run, this);
    }
  }

  virtual void stop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (active_) {
      active_ = false;
      lock.unlock();

      cond_.notify_all();
      thread_.join();
    }
  }

  // the external caller should make sure calling schedule() only while the thread is free
  // otherwise, the scheduled job may lost
  virtual void schedule(std::shared_ptr<RunnableJob> job) {
    std::unique_lock<std::mutex> lock(mutex_);
    job_ = std::move(job);
    lock.unlock();
    cond_.notify_one();
  }

protected:
  void run() {
    for (;;) {
      // if the queue is flushed, stop the loop
      if (queue_.push(id_) < 0) {
        break;
      }
      std::unique_lock<std::mutex> lock(mutex_);
      if (!active_) {
        break;
      }
      cond_.wait(lock);
      // get signaled
      if (!active_) {
        break;
      }
      // check the job
      if (job_) {
        // save the job
        std::shared_ptr<RunnableJob> job = std::move(job_);
        lock.unlock();
        // call the job
        (*job)();
      }
    }
  }

private:
  // a queue of free ids
  const size_t id_;
  MessageQueue<size_t>& queue_;
  bool active_;
  std::shared_ptr<RunnableJob> job_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

/*
 * A queued executor of runnable workers.
 * Instead of queuing the jobs, it queues free threads.
 * The thread could be picked by scheduling a task and called immediately
 */
template <typename RunnableJob>
class ThreadPoolExecutor: public JobExecutor<RunnableJob> {
public:
  ThreadPoolExecutor(size_t thread_num)
  : thread_num_(thread_num), queue_(thread_num) {
  }

  virtual ~ThreadPoolExecutor() {
    // make sure all threads are stopped
    stop();
  }

  virtual void start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (executors_.empty()) {
      for (size_t i = 0; i < thread_num_; ++i) {
        // create an executor with the id and reference to the message queue;
        auto executor = std::make_unique<ThreadExecutor<RunnableJob>>(i, queue_);
        executor->start();
        executors_.push_back(std::move(executor));
      }
    }
  }

  virtual void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!executors_.empty()) {
      queue_.flush();
      executors_.clear(); // will call stop() in destructors
    }
  }

  // the external caller should make sure calling schedule() only while the thread is free
  // otherwise, the scheduled job may lost
  virtual void schedule(std::shared_ptr<RunnableJob> job) {
    size_t id;
    if (queue_.pop(id) < 0) {
      // the queue is flushed
      return;
    }
    executors_[id]->schedule(std::move(job));
  }

private:
  const size_t thread_num_;
  MessageQueue<size_t> queue_;
  std::vector<std::unique_ptr<ThreadExecutor<RunnableJob>>> executors_;
  std::mutex mutex_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_CONCURRENCY_THREAD_POOL_EXECUTOR_H_ */
