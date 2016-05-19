#ifndef SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_
#define SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_

#include <memory>
#include <utility>

#include "parser/parser.h"
#include "service/request_handler.h"
#include "utils/cached_buffer.h"
#include "utils/concurrency/job_executor.h"

namespace redgiant {
class Document;
class FeedDocumentRequest;
class QueryExecutor;

class FeedDocumentHandler: public RequestHandler {
public:
  FeedDocumentHandler(std::unique_ptr<Parser<Document>> parser,
      JobExecutor<FeedDocumentRequest>* pipeline, time_t default_ttl)
  : parser_(std::move(parser)), pipeline_(pipeline),
    default_ttl_(default_ttl), buf_(2 * 1024 * 1024) {
  }

  virtual ~FeedDocumentHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  std::shared_ptr<Parser<Document>> parser_;
  JobExecutor<FeedDocumentRequest>* pipeline_;
  time_t default_ttl_;
  CachedBuffer<char> buf_;
};

class FeedDocumentHandlerFactory: public RequestHandlerFactory {
public:
  FeedDocumentHandlerFactory(std::shared_ptr<ParserFactory<Document>> parser_factory,
      JobExecutor<FeedDocumentRequest>* pipeline, time_t default_ttl = 86400)
  : parser_factory_(std::move(parser_factory)), pipeline_(pipeline),
    default_ttl_(default_ttl) {
  }

  virtual ~FeedDocumentHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(
        new FeedDocumentHandler(parser_factory_->create_parser(), pipeline_, default_ttl_));
  }

private:
  std::shared_ptr<ParserFactory<Document>> parser_factory_;
  JobExecutor<FeedDocumentRequest>* pipeline_;
  time_t default_ttl_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_ */
