#ifndef SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_
#define SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_

#include <memory>

#include "parser/parser.h"
#include "service/request_handler.h"
#include "utils/cached_buffer.h"
#include "utils/concurrency/job_executor.h"

namespace redgiant {
class Document;
struct FeedDocumentJob;

class FeedDocumentHandler: public RequestHandler {
public:
  FeedDocumentHandler(JobExecutor<FeedDocumentJob>* pipeline,
      Parser<Document>* parser, uint32_t default_ttl = 0);

  virtual ~FeedDocumentHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  JobExecutor<FeedDocumentJob>* pipeline_;
  Parser<Document>* parser_;
  uint32_t default_ttl_;
  CachedBuffer<char> buf_;
};

class FeedDocumentHandlerFactory: public RequestHandlerFactory {
public:
  FeedDocumentHandlerFactory(JobExecutor<FeedDocumentJob>* pipeline,
      Parser<Document>* parser, uint32_t default_ttl = 0)
  : pipeline_(pipeline), parser_(parser), default_ttl_(default_ttl) {
  }

  virtual ~FeedDocumentHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(
        new FeedDocumentHandler(pipeline_, parser_, default_ttl_));
  }

private:
  JobExecutor<FeedDocumentJob>* pipeline_;
  Parser<Document>* parser_;
  uint32_t default_ttl_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_ */
