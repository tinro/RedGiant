#include "data/query_request_parser.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data/feature.h"
#include "data/feature_cache.h"
#include "data/feature_space.h"
#include "data/feature_vector.h"
#include "data/query_request.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int QueryRequestParser::parse_json(const rapidjson::Value& root, QueryRequest& output) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "request object does not exist.");
    return -1;
  }

  const char* id = json_try_get_string(root, "id");
  if (!id || !id[0]) {
    LOG_ERROR(logger, "request id missing or empty!");
    return -1;
  }

  output.set_request_id(id);
  LOG_TRACE(logger, "request[%s]: parsing.", id);

  int query_count;
  if (json_try_get_int(root, "count", query_count)) {
    output.set_query_count((size_t)query_count);
  }

  auto features = json_try_get_object(root, "features");
  if (!features) {
    LOG_ERROR(logger, "request[%s]: no features found!", id);
    return -1;
  }

  if (parse_feature_spaces(*features, output) < 0) {
    return -1;
  }

  return 0;
}

int QueryRequestParser::parse_feature_spaces(const rapidjson::Value& root, QueryRequest& request) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
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
