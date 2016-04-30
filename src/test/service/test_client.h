#ifndef SRC_TEST_SERVICE_TEST_CLIENT_H_
#define SRC_TEST_SERVICE_TEST_CLIENT_H_

#include <string>
#include <event2/event.h>
#include <evhttp.h>

namespace redgiant {
class TestClient {
public:
  TestClient() = default;
  ~TestClient() = default;

  std::string response() const { return response_; }

  void request(const char* host, int port, enum evhttp_cmd_type type, const char* uri) {
    struct evhttp_connection *conn;
    struct evhttp_request *req;

    base_ = event_base_new();

    conn = evhttp_connection_base_new(base_, NULL, host, port);
    evhttp_connection_set_timeout(conn, 600);

    // req will be managed by conn
    req = evhttp_request_new(on_request_done_cb, this);
    evhttp_add_header(req->output_headers, "Host", "localhost");
    evhttp_make_request(conn, req, type, uri);

    // loop events
    event_base_dispatch(base_);

    // broke from event_base_dispatch, got response
    evhttp_connection_free(conn);
    event_base_free(base_);
  }

protected:
  static void on_request_done_cb(struct evhttp_request *req, void *arg) {
    ((TestClient*)arg)->on_request_done(req);
  }

  void on_request_done(struct evhttp_request *req) {
    int s = 0;
    char buf[1024];
    if (req) {
      s = evbuffer_remove(req->input_buffer, &buf, sizeof(buf) - 1);
    }
    buf[s] = '\0';
    response_ = buf;
    // terminate event_base_dispatch()
    event_base_loopbreak(base_);
  }

  std::string response_;
  struct event_base* base_;
};
} /* namespace redgiant */

#endif /* SRC_TEST_SERVICE_TEST_CLIENT_H_ */
