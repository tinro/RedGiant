#ifndef SRC_MAIN_RANKING_INTERM_QUERY_H_
#define SRC_MAIN_RANKING_INTERM_QUERY_H_

#include <utility>
#include <vector>

#include "data/feature.h"

namespace redgiant {
class IntermQuery {
public:
  typedef Feature::FeatureId FeatureId;
  typedef double QueryWeight;
  typedef std::vector<std::pair<FeatureId, QueryWeight>> QueryFeatures;

  IntermQuery(QueryFeatures features)
  : features_(std::move(features)) {
  }

  ~IntermQuery() = default;

  const QueryFeatures& get_features() const {
    return features_;
  }

private:
  QueryFeatures features_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_INTERM_QUERY_H_ */
