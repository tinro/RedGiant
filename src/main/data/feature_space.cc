#include "data/feature_space.h"

#include <stdexcept>
#include <string>
#include "utils/logger.h"

DECLARE_LOGGER(logger, __FILE__);

namespace redgiant {

auto FeatureSpace::calculate_feature_id(const std::string& feature_key) const
-> FeatureId {
  static std::hash<std::string> string_hash;

  FeatureId id = 0;
  // use high 8 bits to store space id
  id |= ((FeatureId)space_id_ << kSpaceOffset) & kSpaceMask;

  if (type_ == kString) {
    // Any string literal is valid.
    // For strings, hash the string and get the lower 56 bits.
    id |= (FeatureId)string_hash(feature_key) & kFeatureMask;
  } else if (type_ == kInteger) {
    try {
      // For integers, get the lower 56 bits directly.
      unsigned long long number = stoull(feature_key);
      id |= (FeatureId)number & kFeatureMask;
    } catch (std::invalid_argument& exception) {
      LOG_DEBUG(logger, "feature key %s is invalid in space %s",
          feature_key.c_str(), space_name_.c_str());
      return kInvalidId;
    } catch (std::out_of_range& exception) {
      LOG_DEBUG(logger, "feature key %s is out of range in space %s",
          feature_key.c_str(), space_name_.c_str());
      return kInvalidId;
    }
  }

  LOG_TRACE(logger, "Built feature key %s in space %s to id %016llx",
      feature_key.c_str(), space_name_.c_str(), (unsigned long long int)id);
  return id;
}

} /* namespace redgiant */
