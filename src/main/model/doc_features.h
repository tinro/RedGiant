#ifndef SRC_MAIN_MODEL_DOC_FEATURES_H_
#define SRC_MAIN_MODEL_DOC_FEATURES_H_

#include <string>
#include <utility>

#include "model/feature_group.h"
#include "model/feature_space.h"

namespace redgiant {

class DocFeatures {
public:
  DocFeatures(std::string id)
  : doc_id_(std::move(id)) {
  }

  DocFeatures(const DocFeatures& other) = default;
  DocFeatures(DocFeatures&& other) = default;

  ~DocFeatures() = default;

  const std::string& get_doc_id() const {
    return doc_id_;
  }

  const std::map<std::string, std::shared_ptr<FeatureGroup>>& get_feature_groups() const {
    return feature_groups_;
  }

  std::shared_ptr<FeatureGroup> add_feature_group(const std::string& name, std::shared_ptr<FeatureSpace> space) {
    std::shared_ptr<FeatureGroup> group = std::make_shared<FeatureGroup>(name, std::move(space));
    feature_groups_[name] = group;
    return group;
  }

  std::shared_ptr<FeatureGroup> get_feature_group(const std::string& name) {
    auto iter = feature_groups_.find(name);
    if (iter != feature_groups_.end()) {
      return iter->second;
    } else {
      return nullptr;
    }
  }

private:
  std::string doc_id_;
  std::map<std::string, std::shared_ptr<FeatureGroup>> feature_groups_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_DOC_FEATURES_H_ */
