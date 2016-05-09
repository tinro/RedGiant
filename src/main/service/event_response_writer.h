#ifndef _REDGIANT_SERVICE_EVENT_RESPONSE_WRITER_H_
#define _REDGIANT_SERVICE_EVENT_RESPONSE_WRITER_H_

#include <memory>
#include <event2/buffer.h>
#include <evhttp.h>
#include "service/response_writer.h"

namespace redgiant {
class EventResponseWriter: public ResponseWriter {
public:
  EventResponseWriter(evhttp_request* req);
  virtual ~EventResponseWriter() = default;

  virtual void add_header(const char* key, const char* value);
  virtual void add_body(const void* body, size_t size);
  virtual void send(int status_code, const char* status_msg);
  virtual void send_empty(int status_code, const char* status_msg);

private:
  evhttp_request* ev_req_;
  std::shared_ptr<evbuffer> ev_buf_;
};
} // namespace redgiant

#endif // _REDGIANT_SERVICE_EVENT_RESPONSE_WRITER_H_

