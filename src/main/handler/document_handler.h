#ifndef SRC_MAIN_HANDLER_DOCUMENT_HANDLER_H_
#define SRC_MAIN_HANDLER_DOCUMENT_HANDLER_H_

#include <memory>
#include <utility>

#include "data/parser.h"
#include "service/request_handler.h"
#include "utils/cached_buffer.h"
#include "utils/concurrency/job_executor.h"

namespace redgiant {
class Document;
class DocumentIndexView;

class DocumentHandler: public RequestHandler {
public:
  DocumentHandler(std::unique_ptr<Parser<Document>> parser,
      DocumentIndexView* index_view, unsigned long default_ttl)
  : parser_(std::move(parser)), index_view_(index_view),
    default_ttl_(default_ttl), buf_(2 * 1024 * 1024) {
  }

  virtual ~DocumentHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  std::shared_ptr<Parser<Document>> parser_;
  DocumentIndexView* index_view_;
  unsigned long default_ttl_;
  CachedBuffer<char> buf_;
};

class FeedDocumentHandlerFactory: public RequestHandlerFactory {
public:
  FeedDocumentHandlerFactory(std::shared_ptr<ParserFactory<Document>> parser_factory,
      DocumentIndexView* index_view, unsigned long default_ttl = 86400)
  : parser_factory_(std::move(parser_factory)), index_view_(index_view),
    default_ttl_(default_ttl) {
  }

  virtual ~FeedDocumentHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::unique_ptr<RequestHandler>(
        new DocumentHandler(parser_factory_->create_parser(), index_view_, default_ttl_));
  }

private:
  std::shared_ptr<ParserFactory<Document>> parser_factory_;
  DocumentIndexView* index_view_;
  unsigned long default_ttl_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_DOCUMENT_HANDLER_H_ */
