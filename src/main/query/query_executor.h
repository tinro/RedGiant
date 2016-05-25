#ifndef SRC_MAIN_QUERY_QUERY_EXECUTOR_H_
#define SRC_MAIN_QUERY_QUERY_EXECUTOR_H_

#include <map>
#include <memory>
#include <string>

namespace redgiant {
class QueryRequest;
class QueryResult;

class QueryExecutor {
public:
  QueryExecutor() = default;
  virtual ~QueryExecutor() = default;

  virtual std::unique_ptr<QueryResult> execute(const QueryRequest& request) = 0;
};

class QueryExecutorFactory {
public:
  QueryExecutorFactory() = default;
  virtual ~QueryExecutorFactory() = default;

  virtual std::unique_ptr<QueryExecutor> create_executor() = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_QUERY_QUERY_EXECUTOR_H_ */
