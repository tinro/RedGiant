#include "handler/document_handler.h"

#include <time.h>
#include <string>
#include <memory>
#include <utility>

#include "data/document.h"
#include "data/document_parser.h"
#include "index/document_index_view.h"
#include "service/request_context.h"
#include "service/response_writer.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

using namespace std;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

void DocumentHandler::handle_request(const RequestContext* request, ResponseWriter* response) {
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

  std::shared_ptr<Document> doc = std::make_shared<Document>();
  std::string uuid = request->get_query_param("uuid");
  // if there is no uuid in post content, try to get it from the query param.
  if (!uuid.empty()) {
    LOG_DEBUG(logger, "set document uuid from request: %s", uuid.c_str());
    doc->set_doc_id(uuid);
  }

  buf_.alloc(post_len + 1);
  char* value = buf_.data();
  int ret_len = request->get_content(value, post_len);
  value[ret_len] = '\0';

  int ret = parser_->parse(value, ret_len, *doc);
  buf_.clear();

  if (ret < 0) {
    LOG_ERROR(logger, "parse error");
    response->add_body("parse error\n");
    response->send(400, NULL);
    return;
  }

  std::string ttl_str = request->get_query_param("ttl");
  unsigned long ttl = default_ttl_;
  if (!ttl_str.empty()) {
    ttl = std::stoul(ttl_str);
    if (ttl > 0 && ttl != ULONG_MAX) {
      LOG_DEBUG(logger, "set document ttl from request: %lu", (unsigned long)ttl);
    } else {
      ttl = default_ttl_;
    }
  }
  // expire time is current time plus ttl
  time_t expire_time = time(NULL) + ttl;

  // async update
  index_view_->update_document_async(uuid, expire_time, std::move(doc));

  std::ostringstream os;
  os << R"({"ret":"0", "message":"success"})" << std::endl ;
  response->add_body(os.str());
  response->send(200, NULL);
}

} /* namespace redgiant */
