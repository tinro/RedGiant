#include "data/document_parser.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data/document.h"
#include "data/feature.h"
#include "data/feature_space.h"
#include "data/feature_space_manager.h"
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

  // if uuid is not already set
  if (!output.get_id()) {
    const char* uuid = json_try_get_string(root, "uuid");
    // null or empty
    if (!uuid || !uuid[0]) {
      LOG_ERROR(logger, "document uuid missing or empty!");
      return -1;
    }
    LOG_TRACE(logger, "document[%s]: read uuid from json.", uuid);
    output.set_doc_id(uuid);
  }

  auto features = json_try_get_object(root, "features");
  if (!features) {
    LOG_ERROR(logger, "document[%s]: no features found!", output.get_id_str().c_str());
    return -1;
  }

  if (parse_feature_spaces(*features, output) < 0) {
    return -1;
  }

  return 0;
}

int DocumentParser::parse_feature_spaces(const rapidjson::Value& json, Document& doc) {
  for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
    std::string space_name = it->name.GetString();
    std::shared_ptr<FeatureSpace> space = feature_spaces_->get_space(space_name);
    if (!space) {
      // feature space must be pre-defined
      LOG_WARN(logger, "document[%s]: unknown feature space [%s], ignored.",
          doc.get_id_str().c_str(), space_name.c_str());
    } else {
      LOG_TRACE(logger, "document[%s]: parsing feature space [%s]",
          doc.get_id_str().c_str(), space_name.c_str());

      FeatureVector vec(std::move(space));
      int ret = -1;
      if (it->value.IsObject()) {
        ret = parse_feature_vector_multiple_featuers(it->value, doc, vec);
      } else if (it->value.IsString()){
        ret = parse_feature_vector_single_feature(it->value, doc, vec);
      } else if (it->value.IsNumber()) {
        ret = parse_feature_vector_single_score(it->value, doc, vec);
      }

      if (ret == 0) {
        doc.add_feature_vector(std::move(vec));
      } else {
        LOG_WARN(logger, "document[%s]: cannot parse feature space [%s], ignored.",
            doc.get_id_str().c_str(), space_name.c_str());
      }
    }
  }
  return 0;
}

// e.g. { "publisher" : "cnn" }, json is "cnn"
int DocumentParser::parse_feature_vector_single_feature(const rapidjson::Value& json,
    const Document& doc, FeatureVector& vec) {
  if (!json.IsString()) {
    return -1;
  }

  std::shared_ptr<Feature> feature = vec.get_space().create_feature(json.GetString());
  if (feature) {
    LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
        doc.get_id_str().c_str(), (unsigned long long)feature->get_id(),
        feature->get_key().c_str(), vec.get_space_name().c_str());
    vec.add_feature(std::move(feature), 1.0);
  }
  return 0;
}

// e.g. { "download_count" : 123456 }, json is 123456
int DocumentParser::parse_feature_vector_single_score(const rapidjson::Value& json,
    const Document& doc, FeatureVector& vec) {
  if (!json.IsNumber()) {
    return -1;
  }

  // the feature key is empty, we set it to "0" and it is usually configured to integer type.
  std::shared_ptr<Feature> feature =  vec.get_space().create_feature("0");
  if (feature) {
    LOG_TRACE(logger, "document[%s], created feature %016llx (%s) in feature space [%s]",
        doc.get_id_str().c_str(), (unsigned long long)feature->get_id(),
        feature->get_key().c_str(), vec.get_space_name().c_str());
    vec.add_feature(std::move(feature), json.GetDouble());
  }
  return 0;
}

// e.g. { "favorite_sports" : { "football" : 1.0, "tennis" : 2.0 } }
// json is { "football" : 1.0, "tennis" : 2.0 }
int DocumentParser::parse_feature_vector_multiple_featuers(const rapidjson::Value& json,
    const Document& doc, FeatureVector& vec) {
  if (!json.IsObject()) {
    return -1;
  }

  for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
    if (it->name.IsString() && it->value.IsNumber()) {
      std::shared_ptr<Feature> feature = vec.get_space().create_feature(it->name.GetString());
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
