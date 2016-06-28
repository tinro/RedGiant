#ifndef SRC_MAIN_HANDLER_QUERY_HANDLER_H_
#define SRC_MAIN_HANDLER_QUERY_HANDLER_H_

#include <memory>
#include <utility>

#include "data/parser.h"
#include "query/query_executor.h"
#include "service/request_handler.h"
#include "utils/cached_buffer.h"

namespace redgiant {
class QueryRequest;

class QueryHandler: public RequestHandler {
public:
  QueryHandler(std::unique_ptr<Parser<QueryRequest>> parser, std::unique_ptr<QueryExecutor> executor)
  : parser_(std::move(parser)), executor_(std::move(executor)), buf_(2 * 1024 * 1024) {
  }

  virtual ~QueryHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  std::unique_ptr<Parser<QueryRequest>> parser_;
  std::unique_ptr<QueryExecutor> executor_;
  CachedBuffer<char> buf_;
};

class QueryHandlerFactory: public RequestHandlerFactory {
public:
  QueryHandlerFactory(std::shared_ptr<ParserFactory<QueryRequest>> parser_factory,
      std::shared_ptr<QueryExecutorFactory> executor_factory)
  : parser_factory_(std::move(parser_factory)), executor_factory_(std::move(executor_factory)) {
  }

  virtual ~QueryHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::make_unique<QueryHandler>(parser_factory_->create_parser(),
            executor_factory_->create_executor());
  }

private:
  std::shared_ptr<ParserFactory<QueryRequest>> parser_factory_;
  std::shared_ptr<QueryExecutorFactory> executor_factory_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_QUERY_HANDLER_H_ */
