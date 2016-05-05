#ifndef SRC_MAIN_MODEL_FEATURE_SPACE_H_
#define SRC_MAIN_MODEL_FEATURE_SPACE_H_

#include <string>
#include <utility>

namespace redgiant {

class FeatureSpace {
public:
  typedef uint64_t FeatureId;
  typedef uint32_t SpaceId;
  typedef double Weight;

  enum class Type {
    kString,
    kInteger
  };

  FeatureSpace(std::string name, SpaceId id, Type type)
  : space_name_(std::move(name)), space_id_(id), type_(type) {
  }

  FeatureSpace(const FeatureSpace& other) = default;
  FeatureSpace(FeatureSpace&& other) = default;

  ~FeatureSpace() = default;

  const std::string& get_name() const {
    return space_name_;
  }

  SpaceId get_id() const {
    return space_id_;
  }

  Type get_type() const {
    return type_;
  }

  // for tracing
  std::string get_type_name() const {
    if (type_ == Type::kString) {
      return "string";
    } else {
      return "number";
    }
  }

  FeatureId calculate_feature_id(const std::string& feature_key) const;

private:
  static const FeatureId kSpaceMask = (~(FeatureId)0) << 56;
  static const FeatureId kFeatureMask = (~(FeatureId)0) >> 8;

  std::string space_name_;
  SpaceId space_id_;
  Type type_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_SPACE_H_ */
