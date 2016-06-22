#ifndef SRC_MAIN_DATA_FEATURE_SPACE_MANAGER_H_
#define SRC_MAIN_DATA_FEATURE_SPACE_MANAGER_H_

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "data/feature_space.h"
#include "third_party/rapidjson/document.h"

namespace redgiant {

class FeatureSpaceManager {
public:
  typedef FeatureSpace::SpaceId SpaceId;
  typedef FeatureSpace::SpaceType SpaceType;

  FeatureSpaceManager() = default;
  ~FeatureSpaceManager() = default;

  std::shared_ptr<FeatureSpace> get_space(const std::string& space_name) const {
    // read spaces
    std::shared_lock<std::shared_timed_mutex> lock(mutex_spaces_);
    return get_space_internal(space_name);
  }

  void set_space(std::shared_ptr<FeatureSpace> space) {
    if (space) {
      // write spaces
      std::unique_lock<std::shared_timed_mutex> lock(mutex_spaces_);
      set_space_internal(std::move(space));
    }
  }

  std::shared_ptr<FeatureSpace> create_space(std::string space_name,
      SpaceId space_id, SpaceType type) {
    std::shared_ptr<FeatureSpace> space =
        std::make_shared<FeatureSpace>(space_name, space_id, type);
    // write spaces
    std::unique_lock<std::shared_timed_mutex> lock(mutex_spaces_);
    set_space_internal(space);
    return space;
  }

  int initialize(const rapidjson::Value& config);

private:
  // considered as already synchronized.
  std::shared_ptr<FeatureSpace> get_space_internal(const std::string& space_name) const {
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

private:
  std::unordered_map<std::string, std::shared_ptr<FeatureSpace>> spaces_;
  mutable std::shared_timed_mutex mutex_spaces_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_FEATURE_SPACE_MANAGER_H_ */
