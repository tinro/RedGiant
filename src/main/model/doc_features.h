#ifndef SRC_MAIN_MODEL_DOC_FEATURES_H_
#define SRC_MAIN_MODEL_DOC_FEATURES_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "model/feature_space.h"

namespace redgiant {

class DocFeatures {
public:
  typedef FeatureSpace::FeatureId FeatureId;
  typedef FeatureSpace::SpaceId SpaceId;
  typedef FeatureSpace::Weight Weight;

  typedef std::pair<FeatureId, Weight> FeaturePair;
  typedef std::vector<FeaturePair> FeatureWeights;
  typedef std::map<std::string, FeatureWeights> FeatureWeightsMap;

  DocFeatures(std::string id)
  : doc_id_(std::move(id)) {
  }

  DocFeatures(const DocFeatures& other) = default;
  DocFeatures(DocFeatures&& other) = default;

  ~DocFeatures() = default;

  const std::string& get_doc_id() const {
    return doc_id_;
  }

  const FeatureWeightsMap& get_feature_weights() const {
    return feature_weights_;
  }

  void add_feature_space(const std::string& space_name, FeatureWeights features) {
    feature_weights_[space_name] = std::move(features);
  }

private:
  std::string doc_id_;
  // map from name of a feature space to a list of feature-weight pairs
  FeatureWeightsMap feature_weights_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_DOC_FEATURES_H_ */
