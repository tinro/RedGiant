#include "service/event_response_writer.h"

#include <string.h>

namespace redgiant {

EventResponseWriter::EventResponseWriter(evhttp_request* req)
: ev_req_(req), ev_buf_(evbuffer_new(), evbuffer_free) {
}

void EventResponseWriter::add_header(const char* key, const char* value) {
  evhttp_add_header(evhttp_request_get_output_headers(ev_req_), key, value);
}

void EventResponseWriter::add_body(const void* body, size_t size) {
  if (body && size > 0) {
    evbuffer_add(ev_buf_.get(), body, size);
  }
}

void EventResponseWriter::send(int status_code, const char* status_msg) {
  evhttp_send_reply(ev_req_, status_code, status_msg, ev_buf_.get());
}

void EventResponseWriter::send_empty(int status_code, const char* status_msg) {
  evhttp_send_error(ev_req_, status_code, status_msg);
}

} // namespace redgiant

