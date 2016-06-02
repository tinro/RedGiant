#ifndef SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_
#define SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/feature_space.h"
#include "index/document_query.h"
#include "ranking/ranking_model.h"

namespace redgiant {
class FeatureSpaceManager;

/*
 * The feature mapping model defines a group of mappings
 *  from request to document feature spaces.
 */
class FeatureMappingModel: public RankingModel {
public:
  typedef DocumentQuery::Score Score;
  // Tuple of: [Original space, mapped space, and weight]
  typedef std::tuple<std::shared_ptr<FeatureSpace>, std::shared_ptr<FeatureSpace>, Score> SingleMapping;
  typedef std::unordered_multimap<FeatureSpace::SpaceId, SingleMapping> MappingHashMap;

  FeatureMappingModel() = default;
  virtual ~FeatureMappingModel() = default;

  virtual std::unique_ptr<IntermQuery> process(const QueryRequest& request) const;

  // N-N mapping of feature spaces
  void set_mapping(std::shared_ptr<FeatureSpace> from, std::shared_ptr<FeatureSpace> to, Score weight) {
    FeatureSpace::SpaceId id = from->get_id();
    mappings_.emplace(std::make_pair(id, std::make_tuple(std::move(from), std::move(to), weight)));
  }

private:
  MappingHashMap mappings_;
};

class FeatureMappingModelFactory: public RankingModelFactory {
public:
  struct Mapping {
    std::string from;
    std::string to;
    double weight;
  };

  typedef std::vector<Mapping> MappingVector;

  FeatureMappingModelFactory(std::shared_ptr<FeatureSpaceManager> feature_spaces)
  : feature_spaces_(std::move(feature_spaces)) {
  }

  virtual ~FeatureMappingModelFactory() = default;

  virtual const std::string& get_type() const {
    return type_id_;
  }

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const;

private:
  std::shared_ptr<FeatureSpaceManager> feature_spaces_;
  std::string type_id_ = "mapping";
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_ */
