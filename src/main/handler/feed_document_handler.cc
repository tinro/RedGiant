#include "handler/feed_document_handler.h"

#include <string>
#include <memory>
#include <utility>

#include "model/document.h"
#include "pipeline/feed_document_job.h"
#include "service/request_context.h"
#include "service/response_writer.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

using namespace std;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

FeedDocumentHandler::FeedDocumentHandler(JobExecutor<FeedDocumentJob>* doc_pipeline,
    Parser<Document>* doc_parser, uint32_t doc_expires)
: doc_pipeline_(doc_pipeline), doc_parser_(doc_parser), doc_expires_(doc_expires),
  buf_(2 * 1024 * 1024) {
}

void FeedDocumentHandler::handle_request(const RequestContext* request, ResponseWriter* response) {
  StopWatch watch;
  int method = request->get_method();
  if (method != RequestContext::METHOD_POST) {
    LOG_ERROR(logger, "method is not POST");
    response->add_body("method is not POST\n");
    response->send_done(400, NULL);
    return;
  }

  int ignore_time = atoi(request->get_query_param("ignore_time").c_str());

  int post_len = request->get_post_length();
  if (post_len <= 0) {
    LOG_ERROR(logger, "content is missing");
    response->add_body("content is missing\n");
    response->send_done(400, NULL);
    return;
  }

  buf_.alloc(post_len + 1);
  char* value = buf_.data();
  int ret_len = request->get_post_content(value, post_len);
  value[ret_len] = '\0';

  std::shared_ptr<Document> doc = std::make_shared<Document>();
  int ret = doc_parser_->parse(value, ret_len, *doc);
  buf_.clear();

  if (ret < 0) {
    LOG_ERROR(logger, "parse error");
    response->add_body("parse error\n");
    response->send_done(400, NULL);
    return;
  }

  std::shared_ptr<FeedDocumentJob> job = std::make_shared<FeedDocumentJob>();
  job->expire_time = 0;
  job->doc = std::move(doc);

  // do something here
  doc_pipeline_->schedule(std::move(job));

  std::ostringstream os;
  os << R"({"ret":"0", "message":"success"})" << std::endl ;
  response->add_body(os.str());
  response->send_done(200, NULL);
}

} /* namespace redgiant */
