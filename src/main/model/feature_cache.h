#ifndef SRC_MAIN_MODEL_FEATURE_CACHE_H_
#define SRC_MAIN_MODEL_FEATURE_CACHE_H_

#include <string>
#include <unordered_map>
#include <utility>

#include "model/feature.h"
#include "model/feature_space.h"

namespace redgiant {

class FeatureCache {
public:
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

  std::shared_ptr<Feature> get_feature(const std::string& feature_key) const {
    auto iter = features_.find(feature_key);
    if (iter != features_.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

  void set_feature(const std::string& feature_key, std::shared_ptr<Feature> feature) {
    features_[feature_key] = std::move(feature);
  }

private:
  std::unordered_map<std::string, std::shared_ptr<FeatureSpace>> spaces_;
  std::unordered_map<std::string, std::shared_ptr<Feature>> features_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_CACHE_H_ */
