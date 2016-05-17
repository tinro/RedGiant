#ifndef SRC_MAIN_DATA_QUERY_REQUEST_H_
#define SRC_MAIN_DATA_QUERY_REQUEST_H_

#include <string>
#include <utility>
#include <vector>
#include "utils/stop_watch.h"
#include "model/feature.h"
#include "model/feature_vector.h"

namespace redgiant {
class QueryRequest {
public:
  QueryRequest(const std::string& request_id, size_t query_count,
      std::string model_name, StopWatch watch = StopWatch(), bool debug = false)
  : request_id_(request_id), query_count_(query_count),
    model_name_(std::move(model_name)), watch_(watch), debug_(debug) {
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

  void set_request_id(std::string id) {
    request_id_ = std::move(id);
  }

  size_t get_query_count() const {
    return query_count_;
  }

  void set_query_count(size_t query_count) {
    query_count_ = query_count;
  }

  const std::string& get_model_name() const {
    return model_name_;
  }

  void set_model_name(std::string model_name) {
    model_name_ = std::move(model_name);
  }

  const std::vector<FeatureVector> get_feature_vectors() const {
    return feature_vectors_;
  }

  void add_feature_vector(FeatureVector feature_vector) {
    feature_vectors_.push_back(std::move(feature_vector));
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
  std::string model_name_;
  std::vector<FeatureVector> feature_vectors_;
  StopWatch watch_;
  bool debug_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_QUERY_REQUEST_H_ */
