#include "parser/json_doc_parser.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>
#include <vector>

#include "model/doc_features.h"
#include "model/feature.h"
#include "model/feature_cache.h"
#include "utils/logger.h"

using namespace std;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<DocFeatures> JsonDocParser::parse_json(const rapidjson::Value& root) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "document does not exist.");
    return nullptr;
  }

  std::string uuid;
  if (root["uuid"].IsString()) {
    uuid = root["uuid"].GetString();
  } else {
    LOG_ERROR(logger, "document uuid missing!");
    return nullptr;
  }

  if (uuid.empty()) {
    LOG_ERROR(logger, "document uuid is empty!");
    return nullptr;
  }

  std::unique_ptr<DocFeatures> doc(new DocFeatures(std::move(uuid)));
  LOG_TRACE(logger, "document[%s] created.", uuid.c_str());

  if (root["features"].IsObject()) {
    parse_feature_spaces(root["features"], *doc);
  }
  return doc;
}

void JsonDocParser::parse_feature_spaces(const rapidjson::Value& root, DocFeatures& doc) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString()) {
      std::string space_name = it->name.GetString();
      std::shared_ptr<FeatureSpace> space = cache_->get_space(space_name);
      if (!space) {
        // feature space must be pre-defined in feature cache
        LOG_WARN(logger, "document[%s], unknown feature space %s, ignored.",
            doc.get_doc_id().c_str(), space_name.c_str());
      } else {
        LOG_TRACE(logger, "document[%s], parsing feature space %s",
            doc.get_doc_id().c_str(), space_name.c_str());

        DocFeatures::FeatureWeights features;
        if (it->value.IsObject()) {
          parse_multi_value_feature_space(it->value, doc, space, features);
        } else if (it->value.IsString()){
          parse_single_value_feature_space(it->value, doc, space, features);
        }
        doc.add_feature_space(space_name, std::move(features));
      }
    }
  }
}

// this feature space contains just one string value as its feature key
void JsonDocParser::parse_single_value_feature_space(const rapidjson::Value& root,
    const DocFeatures& doc, const std::shared_ptr<FeatureSpace>& space,
    DocFeatures::FeatureWeights& features) {
  std::shared_ptr<Feature> feature = cache_->create_feature(root.GetString(), space);
  if (feature) {
    LOG_TRACE(logger, "document[%s], created feature %016llx in feature space %s",
        doc.get_doc_id().c_str(), (unsigned long long)feature->get_id(), space->get_name().c_str());
    features.emplace_back(feature->get_id(), 1.0);
  }
}

// parse an object of feature-weight pairs
void JsonDocParser::parse_multi_value_feature_space(const rapidjson::Value& root,
    const DocFeatures& doc, const std::shared_ptr<FeatureSpace>& space,
    DocFeatures::FeatureWeights& features) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString() && it->value.IsNumber()) {
      std::shared_ptr<Feature> feature = cache_->create_feature(it->name.GetString(), space);
      if (feature) {
        LOG_TRACE(logger, "document[%s], created feature %016llx in feature space %s",
            doc.get_doc_id().c_str(), (unsigned long long)feature->get_id(), space->get_name().c_str());
        features.emplace_back(feature->get_id(), it->value.GetDouble());
      }
    }
  }
}

} /* namespace redgiant */
