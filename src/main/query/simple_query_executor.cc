#include "query/simple_query_executor.h"

#include "core/reader/reader_utils.h"
#include "data/query_request.h"
#include "data/query_result.h"
#include "index/document_index_manager.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int SimpleQueryExecutor::execute(const QueryRequest& request, QueryResult& result) {

  result.track_latency(request.get_watch(), QueryResult::kStart);

  std::unique_ptr<DocumentQuery> query = model_->process(request);

  result.track_latency(request.get_watch(), QueryResult::kLoadModel);
  result.track_latency(request.get_watch(), QueryResult::kQueryStart);

  // execute document query
  std::unique_ptr<DocumentIndexManager::Reader> results_reader = index_->query(request, *query);
  size_t query_count = request.get_query_count();

  result.track_latency(request.get_watch(), QueryResult::kQueryExecute);

  if (!results_reader) {
     if (request.is_debug()) {
       LOG_INFO(logger, "[query:%s] received empty document result.", request.get_request_id().c_str());
     }

     result.track_latency(request.get_watch(), QueryResult::kFinalize);
     return 0;
  }

  auto topn_results = read_topn(*results_reader, query_count);

  result.track_latency(request.get_watch(), QueryResult::kQueryRead);

  auto& results = result.get_results();
  for (const auto& r: topn_results) {
    results.emplace_back(r.first.to_string(), r.second);
  }

  result.track_latency(request.get_watch(), QueryResult::kQueryConvert);
  result.track_latency(request.get_watch(), QueryResult::kFinalize);
  return 0;
}

} /* namespace redgiant */
