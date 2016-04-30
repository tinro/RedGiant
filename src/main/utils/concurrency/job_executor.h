#ifndef SRC_MAIN_UTILS_CONCURRENCY_JOB_EXECUTOR_H_
#define SRC_MAIN_UTILS_CONCURRENCY_JOB_EXECUTOR_H_

#include <memory>

namespace redgiant {
/*
 * A simple wrapper interface for asynchronously invoked jobs.
 * Note: the callers must make sure
 * 1. do not call start() and stop() simultaneously.
 *   usually they are called in a same thread so that it stands.
 * 2. OK, seems no other constrains
 */
template<typename Job>
class JobExecutor {
public:
  JobExecutor() = default;
  virtual ~JobExecutor() = default;

  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void schedule(std::shared_ptr<Job> job) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_CONCURRENCY_JOB_EXECUTOR_H_ */
