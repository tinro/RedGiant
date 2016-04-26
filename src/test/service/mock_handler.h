#ifndef REDGIANT_TEST_SERVICE_MOCK_HANDLER_H_
#define REDGIANT_TEST_SERVICE_MOCK_HANDLER_H_

#include <memory>
#include <string>
#include <utility>
#include "service/request_context.h"
#include "service/request_handler.h"
#include "service/response_writer.h"

namespace redgiant {
class MockHandler: public RequestHandler {
public:
  std::string message_;

  MockHandler(std::string message)
  : message_(std::move(message)) {
  }

  virtual ~MockHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response) {
    (void) request;
    response->add_body(message_);
    response->send_done(200, NULL);
  }
};

class MockHandlerFactory: public RequestHandlerFactory {
public:
  std::string message_;

  MockHandlerFactory(std::string message)
  : message_(std::move(message)) {
  }

  virtual ~MockHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(new MockHandler(message_));
  }
};
} /* namespace redgiant */

#endif /* REDGIANT_TEST_SERVICE_MOCK_HANDLER_H_ */
