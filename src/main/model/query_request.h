#ifndef SRC_MAIN_DATA_QUERY_REQUEST_H_
#define SRC_MAIN_DATA_QUERY_REQUEST_H_

#include <string>
#include <utility>
#include <vector>
#include "utils/stop_watch.h"
#include "model/feature.h"

namespace redgiant {
class QueryRequest {
public:
  typedef Feature::FeatureId FeatureId;
  typedef double Weight;
  typedef std::pair<FeatureId, Weight> QueryFeaturePair;

  QueryRequest(const std::string& request_id, size_t query_count,
      std::vector<QueryFeaturePair> query_features,
      StopWatch watch = StopWatch(), bool debug = false)
  : request_id_(request_id), query_count_(query_count),
    query_features_(std::move(query_features)), watch_(watch), debug_(debug) {
  }

  // no copy
  QueryRequest(const QueryRequest&) = delete;
  QueryRequest& operator= (const QueryRequest&) = delete;

  // movable
  QueryRequest(QueryRequest&&) = default;
  QueryRequest& operator= (QueryRequest&&) = default;

  ~QueryRequest() = default;

  const std::string& get_request_id() const {
    return request_id_;
  }

  size_t get_query_count() const {
    return query_count_;
  }

  const std::vector<QueryFeaturePair> get_query_features() const {
    return query_features_;
  }

  const StopWatch& get_watch() const {
    return watch_;
  }

  bool is_debug() const {
    return debug_;
  }

private:
  std::string request_id_;
  size_t query_count_;
  std::vector<QueryFeaturePair> query_features_;
  StopWatch watch_;
  bool debug_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_QUERY_REQUEST_H_ */
