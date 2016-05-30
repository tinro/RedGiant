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

  auto features = json_try_get_object(root, "features");
  if (!features) {
    LOG_ERROR(logger, "request[%s]: no features found!", output.get_request_id().c_str());
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
      LOG_WARN(logger, "request[%s]: unknown feature space [%s], ignored.",
          request.get_request_id().c_str(), space_name.c_str());
    } else {
      LOG_TRACE(logger, "request[%s]: parsing feature space [%s]",
          request.get_request_id().c_str(), space_name.c_str());

      FeatureVector vec(std::move(space));
      int ret = -1;
      // only deal with feature vectors in multiple value format
      if (it->value.IsObject()) {
        ret = parse_feature_vector_multiple_featuers(it->value, request, vec);
      }

      if (ret == 0) {
        request.add_feature_vector(std::move(vec));
      } else {
        LOG_WARN(logger, "request[%s]: cannot parse feature space [%s], ignored.",
            request.get_request_id().c_str(), space_name.c_str());
      }
    }
  }
  return 0;
}

int QueryRequestParser::parse_feature_vector_multiple_featuers(const rapidjson::Value& json,
    const QueryRequest& request, FeatureVector& vec) {
  if (!json.IsObject()) {
    return -1;
  }

  for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
    if (it->name.IsString() && it->value.IsNumber()) {
      std::shared_ptr<Feature> feature = cache_->get_or_create_feature(it->name.GetString(), vec.get_space());
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
