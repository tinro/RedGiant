#ifndef SRC_MAIN_MODEL_FEATURE_H_
#define SRC_MAIN_MODEL_FEATURE_H_

#include <string>
#include <utility>

#include "model/feature_space.h"

namespace redgiant {

/*
 * Immutable feature definition.
 */
class Feature {
public:
  Feature(std::shared_ptr<FeatureSpace> space, std::string key)
  : space_(std::move(space)), key_(std::move(key)) {
    id_ = space_->calculate_feature_id(key_);
  }

  Feature(const Feature& other) = default;
  Feature(Feature&& other) = default;

  ~Feature() = default;

  const std::string& get_key() const {
    return key_;
  }

  const std::string& get_space_name() const {
    return space_->get_name();
  }

  FeatureSpace::FeatureId get_id() const {
    return id_;
  }

private:
  std::shared_ptr<FeatureSpace> space_;
  std::string key_;
  FeatureSpace::FeatureId id_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_H_ */
