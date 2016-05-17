#include "handler/feed_document_handler.h"

#include <time.h>
#include <string>
#include <memory>
#include <utility>

#include "model/document.h"
#include "parser/document_parser.h"
#include "pipeline/feed_document_request.h"
#include "service/request_context.h"
#include "service/response_writer.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

using namespace std;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

void FeedDocumentHandler::handle_request(const RequestContext* request, ResponseWriter* response) {
  StopWatch watch;

  int method = request->get_method();
  if (method != RequestContext::METHOD_PUT) {
    LOG_ERROR(logger, "method is not PUT");
    response->add_body("method is not PUT\n");
    response->send(400, NULL);
    return;
  }

  int post_len = request->get_content_length();
  if (post_len <= 0) {
    LOG_ERROR(logger, "content is missing");
    response->add_body("content is missing\n");
    response->send(400, NULL);
    return;
  }

  buf_.alloc(post_len + 1);
  char* value = buf_.data();
  int ret_len = request->get_content(value, post_len);
  value[ret_len] = '\0';

  std::shared_ptr<Document> doc = std::make_shared<Document>();
  int ret = parser_->parse(value, ret_len, *doc);
  buf_.clear();

  if (ret < 0) {
    LOG_ERROR(logger, "parse error");
    response->add_body("parse error\n");
    response->send(400, NULL);
    return;
  }

  // create feeding request as a job
  std::shared_ptr<FeedDocumentRequest> job =
      std::make_shared<FeedDocumentRequest>(std::move(doc), time(NULL) + default_ttl_, watch);
  // do something here
  pipeline_->schedule(std::move(job));

  std::ostringstream os;
  os << R"({"ret":"0", "message":"success"})" << std::endl ;
  response->add_body(os.str());
  response->send(200, NULL);
}

} /* namespace redgiant */
