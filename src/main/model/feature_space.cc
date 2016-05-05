#include "model/feature_space.h"

#include <string>
#include "utils/logger.h"

DECLARE_LOGGER(logger, __FILE__);

namespace redgiant {

auto FeatureSpace::calculate_feature_id(const std::string& feature_key) const
-> FeatureId {
  static std::hash<std::string> string_hash;

  FeatureId id = 0;
  // use high 8 bits to store space id
  id |= ((FeatureId)space_id_ << 56) & kSpaceMask;

  if (type_ == Type::kString) {
    // For strings, hash the string and get the lower 56 bits.
    id |= (FeatureId)string_hash(feature_key) & kFeatureMask;
  } else {
    // For integers, get the lower 56 bits directly.
    unsigned long number = stoul(feature_key);
    id |= (FeatureId)number & kFeatureMask;
  }

  LOG_TRACE(logger, "Built feature key %s in space %s to id %016llx",
      feature_key.c_str(), space_name_.c_str(), id);
  return id;
}

} /* namespace redgiant */
