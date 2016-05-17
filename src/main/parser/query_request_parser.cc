#include "parser/query_request_parser.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "model/feature.h"
#include "model/feature_cache.h"
#include "model/feature_space.h"
#include "model/feature_vector.h"
#include "model/query_request.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int QueryRequestParser::parse_json(const rapidjson::Value& root, QueryRequest& output) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "request object does not exist.");
    return -1;
  }

  if (!root.HasMember("id") || !root["id"].IsString()) {
    LOG_ERROR(logger, "request id missing!");
    return -1;
  }

  std::string id;
  id = root["id"].GetString();
  if (id.empty()) {
    LOG_ERROR(logger, "request id is empty!");
    return -1;
  }

  output.set_request_id(std::move(id));
  LOG_TRACE(logger, "request[%s]: parsing.", output.get_request_id().c_str());

  int query_count;
  if (root.HasMember("count") && root["count"].IsInt()) {
    if ((query_count = root["count"].GetInt()) > 0) {
      output.set_query_count((size_t)query_count);
    }
  }

  if (!root.HasMember("features") || !root["features"].IsObject()) {
    LOG_ERROR(logger, "request[%s]: no features found!", output.get_request_id().c_str());
    return -1;
  }

  if (parse_feature_spaces(root["features"], output) < 0) {
    return -1;
  }

  return 0;
}

int QueryRequestParser::parse_feature_spaces(const rapidjson::Value& root, QueryRequest& request) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString()) {
      std::string space_name = it->name.GetString();
      std::shared_ptr<FeatureSpace> space = cache_->get_space(space_name);
      if (!space) {
        // feature space must be pre-defined in feature cache
        LOG_WARN(logger, "document[%s]: unknown feature space [%s], ignored.",
            request.get_request_id().c_str(), space_name.c_str());
      } else {
        LOG_TRACE(logger, "document[%s]: parsing feature space [%s]",
            request.get_request_id().c_str(), space_name.c_str());

        FeatureVector vec(std::move(space));
        if (it->value.IsObject()) {
          parse_multi_value_feature_vector(it->value, request, vec);
        } else if (it->value.IsString()){
          parse_single_value_feature_vector(it->value, request, vec);
        }
        request.add_feature_vector(std::move(vec));
      }
    }
  }
  return 0;
}

// this feature space contains just one string value as its feature key
int QueryRequestParser::parse_single_value_feature_vector(const rapidjson::Value& root,
    const QueryRequest& request, FeatureVector& vec) {
  std::shared_ptr<Feature> feature = cache_->get_feature(root.GetString(), *(vec.get_space()));
  if (feature) {
    LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
        request.get_request_id().c_str(), (unsigned long long)feature->get_id(),
        feature->get_key().c_str(), vec.get_space_name().c_str());
    vec.add_feature(std::move(feature), 1.0);
  }
  return 0;
}

// parse an object of feature-weight pairs
int QueryRequestParser::parse_multi_value_feature_vector(const rapidjson::Value& root,
    const QueryRequest& request, FeatureVector& vec) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString() && it->value.IsNumber()) {
      std::shared_ptr<Feature> feature = cache_->get_feature(it->name.GetString(), *(vec.get_space()));
      if (feature) {
        LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
            request.get_request_id().c_str(), (unsigned long long)feature->get_id(),
            feature->get_key().c_str(), vec.get_space_name().c_str());
        vec.add_feature(std::move(feature), it->value.GetDouble());
      }
    }
  }
  return 0;
}
} /* namespace redgiant */
