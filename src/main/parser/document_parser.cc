#include "parser/document_parser.h"

#include <string>
#include <vector>

#include "model/document.h"
#include "model/feature.h"
#include "model/feature_cache.h"
#include "model/feature_vector.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int DocumentParser::parse_json(const rapidjson::Value& root, Document& output) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "document does not exist.");
    return -1;
  }

  if (!root.HasMember("uuid") || !root["uuid"].IsString()) {
    LOG_ERROR(logger, "document uuid missing!");
    return -1;
  }

  std::string uuid;
  uuid = root["uuid"].GetString();
  if (uuid.empty()) {
    LOG_ERROR(logger, "document uuid is empty!");
    return -1;
  }

  output.set_doc_id(std::move(uuid));
  LOG_TRACE(logger, "document[%s]: parsing.", output.get_id_str().c_str());

  if (!root.HasMember("features") || !root["features"].IsObject()) {
    LOG_ERROR(logger, "document[%s]: no features found!", output.get_id_str().c_str());
    return -1;
  }

  if (parse_feature_spaces(root["features"], output) < 0) {
    return -1;
  }

  return 0;
}

int DocumentParser::parse_feature_spaces(const rapidjson::Value& root, Document& doc) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString()) {
      std::string space_name = it->name.GetString();
      std::shared_ptr<FeatureSpace> space = cache_->get_space(space_name);
      if (!space) {
        // feature space must be pre-defined in feature cache
        LOG_WARN(logger, "document[%s]: unknown feature space [%s], ignored.",
            doc.get_id_str().c_str(), space_name.c_str());
      } else {
        LOG_TRACE(logger, "document[%s]: parsing feature space [%s]",
            doc.get_id_str().c_str(), space_name.c_str());

        FeatureVector vec(std::move(space));
        if (it->value.IsObject()) {
          parse_multi_value_feature_vector(it->value, doc, vec);
        } else if (it->value.IsString()){
          parse_single_value_feature_vector(it->value, doc, vec);
        }
        doc.add_feature_vector(std::move(vec));
      }
    }
  }
  return 0;
}

// this feature space contains just one string value as its feature key
int DocumentParser::parse_single_value_feature_vector(const rapidjson::Value& root,
    const Document& doc, FeatureVector& vec) {
  std::shared_ptr<Feature> feature = cache_->create_or_get_feature(root.GetString(), vec.get_space());
  if (feature) {
    LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
        doc.get_id_str().c_str(), (unsigned long long)feature->get_id(),
        feature->get_key().c_str(), vec.get_space_name().c_str());
    vec.add_feature(std::move(feature), 1.0);
  }
  return 0;
}

// parse an object of feature-weight pairs
int DocumentParser::parse_multi_value_feature_vector(const rapidjson::Value& root,
    const Document& doc, FeatureVector& vec) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
    if (it->name.IsString() && it->value.IsNumber()) {
      std::shared_ptr<Feature> feature = cache_->create_or_get_feature(it->name.GetString(), vec.get_space());
      if (feature) {
        LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
            doc.get_id_str().c_str(), (unsigned long long)feature->get_id(),
            feature->get_key().c_str(), vec.get_space_name().c_str());
        vec.add_feature(std::move(feature), it->value.GetDouble());
      }
    }
  }
  return 0;
}

} /* namespace redgiant */
