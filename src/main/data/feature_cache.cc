#include "data/feature_cache.h"

#include <memory>
#include <utility>

#include "data/feature.h"
#include "data/feature_space.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

auto FeatureCache::create_or_get_feature(const std::string& feature_key,
    const std::string& space_name)
-> std::shared_ptr<Feature> {
  shared_lock<shared_mutex> lock(mutex_);
  std::shared_ptr<FeatureSpace> space = get_space_internal(space_name);
  lock.unlock();

  if (space) {
    return create_or_get_feature(feature_key, space);
  }
  return nullptr;
}

auto FeatureCache::create_or_get_feature(const std::string& feature_key,
    const std::shared_ptr<FeatureSpace>& space)
-> std::shared_ptr<Feature> {
  FeatureId id = space->calculate_feature_id(feature_key);
  if (id == FeatureSpace::kInvalidId) {
    return nullptr;
  }

  std::unique_lock<shared_mutex> lock(mutex_);
  auto iter = features_.find(id);
  if (iter != features_.end()) {
    return iter->second;
  } else {
    std::shared_ptr<Feature> feature = std::make_shared<Feature>(feature_key, id);
    // add the newly created feature
    features_[id] = feature;
    return feature;
  }
}

int FeatureCache::initialize(const rapidjson::Value& root) {
  if (!root.IsArray()) {
    LOG_ERROR(logger, "feature spaces should be array!");
    return -1;
  }

  std::unique_lock<shared_mutex> lock(mutex_);
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

    set_space_internal(std::make_shared<FeatureSpace>(
        name, id, type == "string" ? SpaceType::kString : SpaceType::kInteger));
  }
  return 0;

}

} /* namespace redgiant */
