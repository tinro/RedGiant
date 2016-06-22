#ifndef SRC_MAIN_QUERY_SIMPLE_QUERY_EXECUTOR_H_
#define SRC_MAIN_QUERY_SIMPLE_QUERY_EXECUTOR_H_

#include "query/query_executor.h"
#include "ranking/ranking_model.h"

namespace redgiant {
class DocumentIndexManager;

class SimpleQueryExecutor: public QueryExecutor {
public:
  SimpleQueryExecutor(DocumentIndexManager* index, RankingModel* model)
  : index_(index), model_(model) {
  }

  virtual ~SimpleQueryExecutor() = default;

  virtual std::unique_ptr<QueryResult> execute(const QueryRequest& request);

private:
  DocumentIndexManager* index_;
  RankingModel* model_;
};

class SimpleQueryExecutorFactory: public QueryExecutorFactory {
public:
  SimpleQueryExecutorFactory(DocumentIndexManager* index, RankingModel* model)
  : index_(index), model_(model) {
  }

  virtual ~SimpleQueryExecutorFactory() = default;

  virtual std::unique_ptr<QueryExecutor> create_executor() {
    return std::make_unique<SimpleQueryExecutor>(index_, model_);
  }

private:
  DocumentIndexManager* index_;
  RankingModel* model_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_QUERY_SIMPLE_QUERY_EXECUTOR_H_ */
