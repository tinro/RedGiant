#include "service/server_instance.h"

#include <cstring>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <evhttp.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "service/event_request_context.h"
#include "service/event_response_writer.h"
#include "service/request_handler.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

ServerInstance::ServerInstance(int fd, int notify_fd, size_t max_req_per_thread_, RouteMap route_map)
: fd_(fd), notify_fd_(notify_fd), max_req_per_thread_(max_req_per_thread_),
  route_map_(std::move(route_map)), ev_base_(NULL), ev_http_(NULL), handle_count_(0) {
}

ServerInstance::~ServerInstance() {
  if (ev_http_) {
    evhttp_free(ev_http_);
  }
  if (ev_base_) {
    event_base_free(ev_base_);
  }
}

void ServerInstance::on_request_cb(evhttp_request *req, void* arg) {
  ((ServerInstance*)arg)->on_request(req);
}

void ServerInstance::on_request(evhttp_request *req) {
  const char *uri = evhttp_request_get_uri(req);
  if (uri == NULL) {
    LOG_ERROR(logger, "%s - Can't get URI", req->remote_host);
    evhttp_send_error(req, 400, NULL);
    return;
  }

  LOG_DEBUG(logger, "%s - Access: %s", req->remote_host, uri);
  evhttp_uri* ev_uri = evhttp_uri_parse(uri);
  if (ev_uri == NULL) {
    LOG_ERROR(logger, "%s - Failed parse uri", req->remote_host);
    evhttp_send_error(req, 400, NULL);
    return;
  }

  // the lifecycle of ev_uri is now managed by the shared_ptr
  std::shared_ptr<evhttp_uri> ev_uri_sp(ev_uri, evhttp_uri_free);
  const char* path = evhttp_uri_get_path(ev_uri);
  std::string path_str;
  if (path == NULL) {
    path_str = "/";
  } else {
    path_str = path;
  }

  auto find_it = route_map_.find(path_str);
  if (find_it == route_map_.end()) {
    LOG_ERROR(logger, "%s - URI Not Found: %s", req->remote_host, path_str.c_str());
    evhttp_send_error(req, 404, NULL);
    return;
  }

  EventRequestContext request_ctx(req, ev_uri_sp);
  EventResponseWriter response_writer(req);
  // close the connection after such amount of requests
  handle_count_++;
  if (max_req_per_thread_ > 0 && handle_count_ > max_req_per_thread_) {
    handle_count_ = 0;
    response_writer.add_header("Connection", "close");
  }
  // the handler may extend the lifetime of request context and response writer
  find_it->second->handle_request(&request_ctx, &response_writer);
}

void ServerInstance::on_notify_cb(evutil_socket_t fd, short what, void* arg) {
  ((ServerInstance*)arg)->on_notify(fd, what);
}

void ServerInstance::on_notify(evutil_socket_t fd, short what) {
  (void) fd;
  (void) what;
  LOG_DEBUG(logger, "Received server exit notify");
  event_base_loopbreak(ev_base_);
}

int ServerInstance::initialize(size_t id) {
  event_config* ev_config = event_config_new();
  event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK );
  event_config_set_flag(ev_config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST );
  ev_base_ = event_base_new_with_config(ev_config);
  event_config_free(ev_config);
  if (ev_base_ == NULL) {
    LOG_ERROR(logger, "failed event_init");
    return -1;
  }

  if (id == 0) {
    LOG_INFO(logger, "Using Libevent with backend method %s",
        event_base_get_method(ev_base_));
    int f = event_base_get_features(ev_base_);
    if ((f & EV_FEATURE_ET))
      LOG_INFO(logger, "  Edge-triggered events are supported.");
    if ((f & EV_FEATURE_O1))
      LOG_INFO(logger, "  O(1) event notification is supported.");
    if ((f & EV_FEATURE_FDS))
      LOG_INFO(logger, "  All FD types are supported.");
  }

  ev_http_ = evhttp_new(ev_base_);
  if (ev_http_ == NULL) {
    LOG_ERROR(logger, "failed evhttp_new");
    return -1;
  }

  int ret = evhttp_accept_socket(ev_http_, fd_);
  if (ret < 0) {
    LOG_ERROR (logger, "failed accept socket");
    return -1;
  }

  evhttp_set_gencb(ev_http_, on_request_cb, this);

  // this event will be used to notify exit
  struct event *notify_ev = event_new(ev_base_, notify_fd_, EV_TIMEOUT | EV_READ | EV_PERSIST, on_notify_cb, this);
  if (notify_ev == NULL) {
    LOG_ERROR(logger, "failed to add notify event");
    return -1;
  }
  event_add(notify_ev, NULL);
  return 0;
}

int ServerInstance::run() {
  if (ev_base_ == NULL || ev_http_ == NULL) {
    return -1;
  }

  handle_count_ = 0;
  event_base_dispatch(ev_base_);

  LOG_DEBUG(logger, "server instance exit OK");
  return 0;
}

} /* namespace redgiant */
