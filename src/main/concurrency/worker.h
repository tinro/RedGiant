#ifndef SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_H_
#define SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_H_

#include <memory>

namespace redgiant {
template <typename Job>
class Worker {
public:
  Worker() = default;
  virtual ~Worker() = default;

  virtual void prepare() = 0;
  virtual void cleanup() = 0;
  virtual void execute(Job& job) = 0;
};

template <typename Worker>
class WorkerFactory {
public:
  WorkerFactory() = default;
  virtual ~WorkerFactory() = default;

  virtual std::unique_ptr<Worker> create() = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CONCURRENCY_WORKER_H_ */
