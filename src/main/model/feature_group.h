#ifndef SRC_MAIN_MODEL_FEATURE_GROUP_H_
#define SRC_MAIN_MODEL_FEATURE_GROUP_H_

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "model/feature.h"
#include "model/feature_space.h"

namespace redgiant {

class FeatureGroup {
public:
  FeatureGroup(std::string name, std::shared_ptr<FeatureSpace> space)
  : name_(std::move(name)), space_(std::move(space)){
  }

  FeatureGroup(const FeatureGroup& other) = default;
  FeatureGroup(FeatureGroup&& other) = default;

  ~FeatureGroup() = default;

  const std::string& get_name() const {
    return name_;
  }

  std::shared_ptr<FeatureSpace> get_space() const {
    return space_;
  }

  const std::map<FeatureSpace::FeatureId, FeatureSpace::Weight>& get_features() const {
    return weighted_features_;
  }

  FeatureSpace::Weight get_feature_weight(FeatureSpace::FeatureId id) const {
    auto iter = weighted_features_.find(id);
    if (iter != weighted_features_.end()) {
      return iter->second;
    } else {
      return FeatureSpace::Weight(0);
    }
  }

private:
  std::string name_;
  std::shared_ptr<FeatureSpace> space_;
  std::map<FeatureSpace::FeatureId, FeatureSpace::Weight> weighted_features_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_GROUP_H_ */
