#ifndef SRC_MAIN_REDGIANT_SERVICE_SERVER_INSTANCE_H_
#define SRC_MAIN_REDGIANT_SERVICE_SERVER_INSTANCE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <event2/event.h>
#include <evhttp.h>
#include "service/request_handler.h"

namespace redgiant {
class ServerInstance {
public:
  friend class ServerTest;
  friend class ServerInstanceTest;

  typedef std::unordered_map<std::string, std::unique_ptr<RequestHandler>> RouteMap;

  ServerInstance(int fd, int notify_fd, size_t max_req_per_thread, RouteMap route_map);
  ~ServerInstance();

  int initialize(size_t id);
  int run();

private:
  static void on_request_cb(evhttp_request *req, void* arg);
  void on_request(evhttp_request *req);

  static void on_notify_cb(evutil_socket_t fd, short what, void* arg);
  void on_notify(evutil_socket_t fd, short what);

  static bool is_valid_client(const char* remote_addr, ev_uint16_t remote_port);

private:
  int fd_;
  int notify_fd_;
  size_t max_req_per_thread_;
  RouteMap route_map_;

  event_base* ev_base_;
  evhttp* ev_http_;
  size_t handle_count_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_SERVICE_SERVER_INSTANCE_H_ */
