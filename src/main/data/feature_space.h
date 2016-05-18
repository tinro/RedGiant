#ifndef SRC_MAIN_DATA_FEATURE_SPACE_H_
#define SRC_MAIN_DATA_FEATURE_SPACE_H_

#include <memory>
#include <string>
#include <utility>

#include "data/feature.h"

namespace redgiant {

class FeatureSpace {
public:
  typedef Feature::FeatureId FeatureId;
  typedef uint32_t SpaceId;

  enum FeatureType {
    kString,
    kInteger
  };

  FeatureSpace(std::string name, SpaceId id, FeatureType type)
  : space_name_(std::move(name)), space_id_(id), type_(type) {
    // TODO: id should be in range [0,254] (255 is reserved for invalid)
  }

  FeatureSpace(const FeatureSpace&) = default;
  FeatureSpace(FeatureSpace&&) = default;
  ~FeatureSpace() = default;

  const std::string& get_name() const {
    return space_name_;
  }

  SpaceId get_id() const {
    return space_id_;
  }

  FeatureType get_type() const {
    return type_;
  }

  // for tracing
  std::string get_type_name() const {
    if (type_ == kString) {
      return "string";
    } else {
      return "number";
    }
  }

  std::shared_ptr<Feature> create_feature(std::string feature_key) const {
    FeatureId id = calculate_feature_id(feature_key);
    if (id == kInvalidId) {
      return nullptr;
    }
    return std::make_shared<Feature>(std::move(feature_key), id);
  }

  FeatureId calculate_feature_id(const std::string& feature_key) const;

  FeatureId project_to_space(FeatureId id) const {
    return (id & kFeatureMask) | ((FeatureId)space_id_ << kSpaceOffset);
  }

  static SpaceId get_part_space_id(FeatureId id) {
    return (id & kSpaceMask) >> kSpaceOffset;
  }

  static FeatureId get_part_feature_id(FeatureId id) {
    return id & kFeatureMask;
  }

public:
  static const FeatureId kInvalidId = (~(FeatureId)0); // -1

private:
  static const int kSpaceBits = 8;
  static const int kSpaceOffset = 56;
  static const int kFeatureBits = 56;

  static const FeatureId kSpaceMask = (((FeatureId)1 << kSpaceBits) - 1) << kSpaceOffset;
  static const FeatureId kFeatureMask = ((FeatureId)1 << kFeatureBits) - 1;

  std::string space_name_;
  SpaceId space_id_;
  FeatureType type_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_FEATURE_SPACE_H_ */
