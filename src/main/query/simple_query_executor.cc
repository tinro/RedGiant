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
  result->track_latency(QueryResult::kQueryRead);

  auto& results = result->get_results();
  for (const auto& r: topn_results) {
    results.emplace_back(r.first.to_string(), r.second);
  }
  result->track_latency(QueryResult::kResultConvert);
  result->track_latency(QueryResult::kFinalize);

  return result;
}

} /* namespace redgiant */
