#ifndef SRC_MAIN_MODEL_FEATURE_H_
#define SRC_MAIN_MODEL_FEATURE_H_

#include <string>
#include <memory>
#include <utility>

namespace redgiant {

/*
 * Immutable feature definition.
 */
class Feature {
public:
  typedef uint64_t FeatureId;

  Feature(std::string key, FeatureId id)
  : key_(std::move(key)), id_(id) {
  }

  Feature(const Feature&) = default;
  Feature(Feature&&) = default;
  ~Feature() = default;

  const std::string& get_key() const {
    return key_;
  }

  FeatureId get_id() const {
    return id_;
  }

private:
  std::string key_;
  FeatureId id_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_MODEL_FEATURE_H_ */
