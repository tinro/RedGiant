#ifndef _REDGIANT_SERVICE_REQUEST_HANDLER_H_
#define _REDGIANT_SERVICE_REQUEST_HANDLER_H_

#include <memory>

namespace redgiant {
class RequestContext;
class ResponseWriter;

class RequestHandler {
public:
  RequestHandler() = default;
  virtual ~RequestHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response) = 0;
};

class RequestHandlerFactory {
public:
  RequestHandlerFactory() = default;
  virtual ~RequestHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() = 0;
};
} // namespace redgiant

#endif // _REDGIANT_SERVICE_REQUEST_HANDLER_H_
