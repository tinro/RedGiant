#include "parser/document_parser.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data/document.h"
#include "data/feature.h"
#include "data/feature_cache.h"
#include "data/feature_space.h"
#include "data/feature_vector.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int DocumentParser::parse_json(const rapidjson::Value& root, Document& output) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "document does not exist.");
    return -1;
  }

  const char* uuid = json_try_get_string(root, "uuid");
  // null or empty
  if (!uuid || !uuid[0]) {
    LOG_ERROR(logger, "document uuid missing or empty!");
    return -1;
  }

  output.set_doc_id(uuid);
  LOG_TRACE(logger, "document[%s]: parsing.", uuid);

  auto features = json_try_get_object(root, "features");
  if (!features) {
    LOG_ERROR(logger, "document[%s]: no features found!", uuid);
    return -1;
  }

  if (parse_feature_spaces(*features, output) < 0) {
    return -1;
  }

  return 0;
}

int DocumentParser::parse_feature_spaces(const rapidjson::Value& root, Document& doc) {
  for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
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
