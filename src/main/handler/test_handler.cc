#include "handler/test_handler.h"

#include <iostream>
#include <string>
#include "service/request_context.h"
#include "service/response_writer.h"

using namespace std;

namespace redgiant {

void TestHandler::handle_request(const RequestContext* request, ResponseWriter* response) {
  (void) request;

  response->add_body("OK\n");
  response->send(200, NULL);
}

} /* namespace redgiant */
