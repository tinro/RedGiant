#ifndef REDGIANT_HANDLER_TEST_HANDLER_H_
#define REDGIANT_HANDLER_TEST_HANDLER_H_

#include <memory>
#include "service/request_handler.h"

namespace redgiant {
class TestHandler: public RequestHandler {
public:
  TestHandler() = default;
  virtual ~TestHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);
};

class TestHandlerFactory: public RequestHandlerFactory {
public:
  TestHandlerFactory() = default;
  virtual ~TestHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(new TestHandler());
  }
};
} /* namespace redgiant */

#endif /* REDGIANT_HANDLER_TEST_HANDLER_H_ */
