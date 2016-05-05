#ifndef SRC_MAIN_MODEL_FEATURE_H_
#define SRC_MAIN_MODEL_FEATURE_H_

#include <string>
#include <memory>
#include <utility>

#include "model/feature_space.h"

namespace redgiant {

/*
 * Immutable feature definition.
 */
class Feature {
public:
  typedef FeatureSpace::FeatureId FeatureId;

  Feature(std::shared_ptr<FeatureSpace> space, std::string key)
  : space_(std::move(space)), key_(std::move(key)) {
    id_ = space_->calculate_feature_id(key);
  }

  Feature(const Feature& other) = default;
  Feature(Feature&& other) = default;

  ~Feature() = default;

  std::shared_ptr<FeatureSpace> get_space() const {
    return space_;
  }

  const std::string& get_space_name() const {
    return space_->get_name();
  }

  const std::string& get_key() const {
    return key_;
  }

  FeatureId get_id() const {
    return id_;
  }

private:
  std::shared_ptr<FeatureSpace> space_;
  std::string key_;
  FeatureId id_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_H_ */
