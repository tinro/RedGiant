#ifndef SRC_MAIN_MODEL_FEATURE_VECTOR_H_
#define SRC_MAIN_MODEL_FEATURE_VECTOR_H_

#include <memory>
#include <utility>
#include <vector>

#include "model/feature.h"
#include "model/feature_space.h"

namespace redgiant {
/*
 * A group of weighted features in the given feature space
 */
class FeatureVector {
public:
  typedef double Weight;
  typedef std::pair<std::shared_ptr<Feature>, Weight> FeaturePair;

  FeatureVector(std::shared_ptr<FeatureSpace> space)
  : space_(std::move(space)) {
  }

  FeatureVector(const FeatureVector& other) = default;
  FeatureVector(FeatureVector&& other) = default;

  ~FeatureVector() = default;

  void add_feature(std::shared_ptr<Feature> feature, Weight weight) {
    features_.emplace_back(std::move(feature), weight);
  }

  void add_feature(const std::string& feature_key, Weight weight) {
    features_.emplace_back(space_->create_feature(feature_key), weight);
  }

  const std::shared_ptr<FeatureSpace>& get_space() const {
    return space_;
  }

  const std::string& get_space_name() const {
    return space_->get_name();
  }

  const std::vector<FeaturePair>& get_features() const {
    return features_;
  }

private:
  std::shared_ptr<FeatureSpace> space_;
  std::vector<FeaturePair> features_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_VECTOR_H_ */
