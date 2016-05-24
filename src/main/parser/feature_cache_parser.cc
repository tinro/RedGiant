#include "parser/feature_cache_parser.h"

#include <string>

#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int FeatureCacheParser::parse_json(const rapidjson::Value& root, FeatureCache& output) {
  if (!root.IsArray()) {
    LOG_ERROR(logger, "feature spaces should be array!");
    return -1;
  }


  for (auto it = root.Begin(); it != root.End(); ++it) {
    int id;
    std::string name;
    std::string type;
    if (!json_try_get_int(*it, "id", id)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }

    if (!json_try_get_string(*it, "name", name)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }

    if (!json_try_get_string(*it, "type", type)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }

    output.create_space(name, (FeatureSpace::SpaceId)id,
        type == "string" ? FeatureSpace::kString : FeatureSpace::kInteger);
  }
  return 0;
}

} /* namespace redgiant */
