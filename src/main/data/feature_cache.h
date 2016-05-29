#ifndef SRC_MAIN_DATA_FEATURE_CACHE_H_
#define SRC_MAIN_DATA_FEATURE_CACHE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "data/feature.h"
#include "data/feature_space.h"
#include "third_party/lock/shared_lock.h"
#include "third_party/lock/shared_mutex.h"
#include "third_party/rapidjson/document.h"

namespace redgiant {

class FeatureCache {
public:
  typedef Feature::FeatureId FeatureId;
  typedef FeatureSpace::SpaceId SpaceId;
  typedef FeatureSpace::SpaceType SpaceType;

  FeatureCache() = default;
  ~FeatureCache() = default;

  std::shared_ptr<FeatureSpace> get_space(const std::string& space_name) const {
    shared_lock<shared_mutex> lock(mutex_);
    return get_space_internal(space_name);
  }

  void set_space(std::shared_ptr<FeatureSpace> space) {
    if (space) {
      std::unique_lock<shared_mutex> lock(mutex_);
      set_space_internal(std::move(space));
    }
  }

  std::shared_ptr<FeatureSpace> create_space(std::string space_name,
      SpaceId space_id, SpaceType type) {
    std::shared_ptr<FeatureSpace> space =
        std::make_shared<FeatureSpace>(space_name, space_id, type);
    std::unique_lock<shared_mutex> lock(mutex_);
    set_space_internal(space);
    return space;
  }

  std::shared_ptr<Feature> get_feature(FeatureId id) const {
    shared_lock<shared_mutex> lock(mutex_);
    return get_feature_internal(id);
  }

  std::shared_ptr<Feature> get_feature(const std::string& feature_key,
      const FeatureSpace& space) const {
    shared_lock<shared_mutex> lock(mutex_);
    return get_feature_internal(space.calculate_feature_id(feature_key));
  }

  std::shared_ptr<Feature> get_feature(const std::string& feature_key,
      const std::string& space_name) const {
    shared_lock<shared_mutex> lock(mutex_);
    std::shared_ptr<FeatureSpace> space = get_space_internal(space_name);
    if (space) {
      return get_feature_internal(space->calculate_feature_id(feature_key));
    }
    return nullptr;
  }

  void set_feature(std::shared_ptr<Feature> feature) {
    if (feature) {
      std::unique_lock<shared_mutex> lock(mutex_);
      set_feature_internal(std::move(feature));
    }
  }

  std::shared_ptr<Feature> get_or_create_feature(const std::string& feature_key,
      const std::string& space_name);

  std::shared_ptr<Feature> get_or_create_feature(const std::string& feature_key,
      const FeatureSpace& space);

  int initialize(const rapidjson::Value& config);

private:
  // considered as already synchronized.
  const std::shared_ptr<FeatureSpace> get_space_internal(const std::string& space_name) const {
    auto iter = spaces_.find(space_name);
    if (iter != spaces_.end()) {
      return iter->second;
    }
    return nullptr;
  }

  // considered as already synchronized, and non-empty.
  void set_space_internal(std::shared_ptr<FeatureSpace> space) {
    spaces_[space->get_name()] = std::move(space);
  }

  // considered as already synchronized.
  std::shared_ptr<Feature> get_feature_internal(FeatureId id) const {
    auto iter = features_.find(id);
    if (iter != features_.end()) {
      return iter->second;
    }
    return nullptr;
  }

  // considered as already synchronized, and non-empty.
  void set_feature_internal(std::shared_ptr<Feature> feature) {
    features_[feature->get_id()] = std::move(feature);
  }

private:
  mutable shared_mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<FeatureSpace>> spaces_;
  std::unordered_map<FeatureId, std::shared_ptr<Feature>> features_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_FEATURE_CACHE_H_ */
