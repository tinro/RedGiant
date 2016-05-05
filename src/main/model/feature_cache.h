#ifndef SRC_MAIN_MODEL_FEATURE_CACHE_H_
#define SRC_MAIN_MODEL_FEATURE_CACHE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "model/feature.h"
#include "model/feature_space.h"

namespace redgiant {

class FeatureCache {
public:
  typedef FeatureSpace::FeatureId FeatureId;
  typedef FeatureSpace::SpaceId SpaceId;

  FeatureCache();
  ~FeatureCache() = default;

  std::shared_ptr<FeatureSpace> get_space(const std::string& space_name) const {
    auto iter = spaces_.find(space_name);
    if (iter != spaces_.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

  void set_space(const std::string& space_name, std::shared_ptr<FeatureSpace> space) {
    spaces_[space_name] = std::move(space);
  }

  std::shared_ptr<Feature> get_feature(FeatureId id) const {
    auto iter = features_.find(id);
    if (iter != features_.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

  std::shared_ptr<Feature> get_feature(const std::string& feature_key,
      const FeatureSpace& space) const {
    return get_feature(space.calculate_feature_id(feature_key));
  }

  std::shared_ptr<Feature> get_feature(const std::string& feature_key,
      const std::string& space_name) const {
    auto iter = spaces_.find(space_name);
    if (iter != spaces_.end()) {
      return get_feature(feature_key, *(iter->second));
    } else {
      return nullptr;
    }
  }

  void set_feature(std::shared_ptr<Feature> feature) {
    features_[feature->get_id()] = std::move(feature);
  }

  std::shared_ptr<Feature> create_or_get_feature(const std::string& feature_key,
      const std::string& space_name);

  std::shared_ptr<Feature> create_or_get_feature(const std::string& feature_key,
      const std::shared_ptr<FeatureSpace>& space);

private:
  std::unordered_map<std::string, std::shared_ptr<FeatureSpace>> spaces_;
  std::unordered_map<FeatureId, std::shared_ptr<Feature>> features_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_CACHE_H_ */
