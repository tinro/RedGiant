#include "service/server.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "service/request_handler.h"
#include "service/server_instance.h"
#include "utils/logger.h"

using namespace std;

DECLARE_LOGGER(logger, __FILE__);

namespace redgiant {
Server::Server(int port, size_t thread_num, size_t max_req_per_thread)
: port_(port), thread_num_(thread_num), max_req_per_thread_(max_req_per_thread) {
}

Server::~Server() {
  if (fd_ != -1) {
    ::close(fd_);
  }
  if (notify_fd_[0] != -1) {
    ::close(notify_fd_[0]);
    ::close(notify_fd_[1]);
  }
}

int Server::bind(const std::string& uri, std::shared_ptr<RequestHandlerFactory> factory) {
  auto ret = routes_.emplace(uri, std::move(factory));
  if (!ret.second) {
    LOG_INFO(logger, "bind service end point %s failed! found duplicated items", uri.c_str());
    return -1;
  }
  return 0;
}

int Server::initialize() {
  int backlog = 10240;
  LOG_INFO(logger, "Starting server at port=%d, worker threads=%zu, max_req_per_thread=%zu",
      port_, thread_num_, max_req_per_thread_);

  fd_ = bind_socket(port_, backlog);
  if (fd_ < 0) {
    LOG_ERROR(logger, "failed bind socket");
    return -1;
  }

  if(socketpair(AF_UNIX, SOCK_STREAM, 0, notify_fd_) == -1){
    LOG_ERROR(logger, "failed create signal socket");
    ::close(fd_);
    fd_ = -1;
    notify_fd_[0] = -1;
    notify_fd_[1] = -1;
    return -1;
  }

  int ret = 0;
  for (size_t i = 0; i < thread_num_; ++i) {
    std::unordered_map<std::string, std::unique_ptr<RequestHandler>> route_map;
    for (auto& factory: routes_) {
      route_map.emplace(factory.first, factory.second->create_handler());
    }
    std::shared_ptr<ServerInstance> instance(new ServerInstance(
        fd_, notify_fd_[0], max_req_per_thread_, std::move(route_map)));
    ret = instance->initialize(i);
    if (ret < 0) {
      break;
    }
    instances_.push_back(std::move(instance));
  }

  if (ret < 0) {
    LOG_ERROR(logger, "failed during initialize server context");
    instances_.clear();
    ::close(fd_);
    fd_ = -1;
    ::close(notify_fd_[0]);
    notify_fd_[0] = -1;
    ::close(notify_fd_[1]);
    notify_fd_[1] = -1;
    return -1;
  }
  return 0;
}

int Server::start() {
  if (fd_ < 0 || notify_fd_[1] < 0) {
    LOG_ERROR(logger, "Failed to start server, fd error.");
    return -1;
  }

  LOG_DEBUG(logger, "Server at port=%d init done, starting handler threads.", port_);
  for (auto& instance: instances_) {
    instance_threads_.emplace_back(&ServerInstance::run, instance);
  }

  LOG_INFO(logger, "Server at port=%d started.", port_);
  return 0;
}

int Server::stop() {
  if (fd_ < 0 || notify_fd_[1] < 0) {
    LOG_ERROR(logger, "Failed to stop server, fd error.");
    return -1;
  }

  char buf[] = "stop";
  if (::write(notify_fd_[1], buf, sizeof(buf)) < 0) {
    LOG_ERROR(logger, "Failed to notify server exit, write error.");
    return -1;
  }

  for (auto& thrd: instance_threads_) {
    thrd.join();
  }
  instance_threads_.clear();
  instances_.clear();

  ::close(fd_);
  fd_ = -1;
  ::close(notify_fd_[0]);
  notify_fd_[0] = -1;
  ::close(notify_fd_[1]);
  notify_fd_[1] = -1;

  LOG_INFO(logger, "Server at port=%d stopped.", port_);
  return 0;
}

int Server::bind_socket(int port, int backlog) {
  int ret;
  int nfd;
  nfd = socket(AF_INET, SOCK_STREAM, 0);
  if (nfd < 0) {
    LOG_ERROR(logger, "create socket error");
    return -1;
  }

  int one = 1;
  ret = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  ret = ::bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret < 0) {
    LOG_ERROR (logger, "bind to port %d failed", port);
    return -1;
  }

  ret = ::listen(nfd, backlog);
  if (ret < 0) {
    LOG_ERROR (logger, "listen failed");
    return -1;
  }

  int flags;
  if ((flags = fcntl(nfd, F_GETFL, 0)) < 0
    || fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0) {
    LOG_ERROR (logger, "change to nonblocking failed");
    return -1;
  }
  return nfd;
}

} // namespace redgiant

