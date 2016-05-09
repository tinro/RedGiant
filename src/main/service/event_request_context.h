#ifndef _REDGIANT_SERVICE_EVENT_REQUEST_CONTEXT_H_
#define _REDGIANT_SERVICE_EVENT_REQUEST_CONTEXT_H_

#include <memory>
#include <event2/event.h>
#include <evhttp.h>
#include "service/request_context.h"

namespace redgiant {
class EventRequestContext: public RequestContext {
public:
  EventRequestContext(evhttp_request* req);
  virtual ~EventRequestContext() = default;

  virtual std::string get_uri() const;
  virtual std::string get_path() const;
  virtual std::string get_header(const char* key) const;
  virtual std::string get_query_param(const char* key) const;
  virtual std::map<std::string, std::string> get_query_params() const;
  virtual int get_method() const;
  virtual int get_content_length() const;
  virtual int get_content(char* out_buf, int max_len) const;

private:
  evhttp_request* ev_req_;
  std::shared_ptr<const evhttp_uri> ev_uri_;
};
} // namespace redgiant

#endif // _REDGIANT_SERVICE_EVENT_REQUEST_CONTEXT_H_

