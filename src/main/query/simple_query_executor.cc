#include "query/simple_query_executor.h"

#include "core/reader/reader_utils.h"
#include "data/interm_query.h"
#include "data/query_request.h"
#include "data/query_result.h"
#include "index/document_index_manager.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<QueryResult> SimpleQueryExecutor::execute(const QueryRequest& request) {
  std::unique_ptr<QueryResult> result = request.create_result();
  // tracking latency of each steps of a query request.
  result->track_latency(QueryResult::kStart);

  std::unique_ptr<IntermQuery> interm_query = model_->process(request);
  if (!interm_query) {
    // query empty, which means some error happens to the ranking model.
    if (request.is_debug()) {
       LOG_INFO(logger, "[query:%s] ranking model not found!", request.get_request_id().c_str());
    }
    result->set_error_status(true);
    result->track_latency(QueryResult::kFinalize);
    return result;
  }

  if (request.is_debug()) {
    LOG_INFO(logger, "[query:%s] searching %zu features built by ranking model.",
        request.get_request_id().c_str(), interm_query->get_features().size());
  }
  result->track_latency(QueryResult::kLoadModel);

  DocumentQuery query(request, *interm_query);
  result->track_latency(QueryResult::kBuildQuery);
  result->track_latency(QueryResult::kQueryStart);

  // execute document query
  std::unique_ptr<DocumentIndexManager::Reader> results_reader = index_->query(request, query);
  size_t query_count = request.get_query_count();
  result->track_latency(QueryResult::kQueryExecute);

  if (!results_reader) {
    if (request.is_debug()) {
      LOG_INFO(logger, "[query:%s] received empty document result.", request.get_request_id().c_str());
    }
    result->track_latency(QueryResult::kFinalize);
    return result;
  }

  auto topn_results = read_topn(*results_reader, query_count);
  if (request.is_debug()) {
    size_t n = 0;
    for (const auto& r: topn_results) {
      LOG_INFO(logger, "[query:%s] result %zu: id:%s, score:%lf.",
          request.get_request_id().c_str(), n++, r.first.to_string().c_str(), r.second);
    }
  }
  result->track_latency(QueryResult::kQueryRead);

  for (const auto& r: topn_results) {
    result->get_results().emplace_back(r.first.to_string(), r.second);
  }
  result->track_latency(QueryResult::kResultConvert);
  result->track_latency(QueryResult::kFinalize);

  return result;
}

} /* namespace redgiant */
