#ifndef _REDGIANT_SERVICE_SERVER_H_
#define _REDGIANT_SERVICE_SERVER_H_

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "service/request_handler.h"
#include "service/server_instance.h"

namespace redgiant {
class Server {
public:
  friend class ServerTest;

  typedef std::unordered_map<std::string, std::shared_ptr<RequestHandlerFactory>> RouteFactoryMap;

  Server(int port, size_t thread_num, size_t max_req_per_thread);
  ~Server();

public:
  int bind(const std::string& uri, std::shared_ptr<RequestHandlerFactory> factory);
  int initialize();
  int start();
  int stop();

private:
  static int bind_socket(int port, int backlog);

private:
  int port_;
  size_t thread_num_;
  size_t max_req_per_thread_;
  RouteFactoryMap routes_;
  std::vector<std::shared_ptr<ServerInstance>> instances_;
  std::vector<std::thread> instance_threads_;
  int fd_ = -1;
  int notify_fd_[2] { -1, -1 };
};
} // namespace redgiant

#endif // _REDGIANT_SERVICE_SERVER_H_
