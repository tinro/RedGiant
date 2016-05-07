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
  FeedDocumentHandler(JobExecutor<FeedDocumentJob>* doc_pipeline,
      Parser<Document>* doc_parser, uint32_t doc_expires);

  virtual ~FeedDocumentHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  JobExecutor<FeedDocumentJob>* doc_pipeline_;
  Parser<Document>* doc_parser_;
  uint32_t doc_expires_;
  CachedBuffer<char> buf_;
};

class FeedDocumentHandlerFactory: public RequestHandlerFactory {
public:
  FeedDocumentHandlerFactory(JobExecutor<FeedDocumentJob>* doc_pipeline,
      Parser<Document>* doc_parser, uint32_t doc_expires)
  : doc_pipeline_(doc_pipeline), doc_parser_(doc_parser),
    doc_expires_(doc_expires) {
  }

  virtual ~FeedDocumentHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(
        new FeedDocumentHandler(doc_pipeline_, doc_parser_, doc_expires_));
  }

private:
  JobExecutor<FeedDocumentJob>* doc_pipeline_;
  Parser<Document>* doc_parser_;
  uint32_t doc_expires_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_FEED_DOCUMENT_HANDLER_H_ */
