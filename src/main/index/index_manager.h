#ifndef INDEX_MANAGER_H_
#define INDEX_MANAGER_H_

#include <condition_variable>
#include <ctime>
#include <mutex>
#include <thread>

namespace redgiant {
class IndexManager {
public:
  IndexManager();

  virtual ~IndexManager();

  virtual int start_maintain(int startup_latency, int apply_interval);

  virtual int stop_maintain();

  virtual int do_maintain(time_t time) = 0;

protected:
  void maintain_loop(int startup_latency, int apply_interval);

  // for maintainance
  bool maintain_active_;
  std::thread maintain_thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
};
} /* namespace redgiant */

#endif /* INDEX_MANAGER_H_ */
