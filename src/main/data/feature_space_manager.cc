#include "data/feature_space_manager.h"

#include <memory>
#include <utility>

#include "data/feature_space.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

int FeatureSpaceManager::initialize(const rapidjson::Value& root) {
  if (!root.IsArray()) {
    LOG_ERROR(logger, "feature spaces should be array!");
    return -1;
  }

  // lock write during the whole parsing stage. it will only happen once on startup.
  std::unique_lock<std::shared_timed_mutex> lock(mutex_spaces_);
  for (auto it = root.Begin(); it != root.End(); ++it) {
    int id;
    std::string name;
    std::string type;
    if (!json_try_get_value(*it, "id", id)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }
    if (!json_try_get_value(*it, "name", name)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }
    if (!json_try_get_value(*it, "type", type)) {
      LOG_ERROR(logger, "feature spaces does not contain valid id!");
      return -1;
    }
    set_space_internal(std::make_shared<FeatureSpace>(
        name, id, type == "string" ? SpaceType::kString : SpaceType::kInteger));
  }
  return 0;

}

} /* namespace redgiant */
