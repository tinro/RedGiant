#ifndef SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_
#define SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/feature_cache.h"
#include "data/feature_space.h"
#include "index/document_query.h"
#include "ranking/ranking_model.h"

namespace redgiant {
/*
 * The feature mapping model defines a group of mappings
 *  from request to document feature spaces.
 */
class FeatureMappingModel: public RankingModel {
public:
  typedef DocumentQuery::Score Score;
  // Tuple of: [Original space, mapped space, and weight]
  typedef std::tuple<std::shared_ptr<FeatureSpace>, std::shared_ptr<FeatureSpace>, Score> SingleMapping;
  typedef std::unordered_map<std::string, SingleMapping> MappingHashMap;

  FeatureMappingModel() = default;
  virtual ~FeatureMappingModel() = default;

  virtual std::unique_ptr<IntermQuery> process(const QueryRequest& request) const;

  void set_mapping(std::shared_ptr<FeatureSpace> from, std::shared_ptr<FeatureSpace> to, Score weight) {
    const std::string& name = from->get_name();
    mappings_[name] = std::make_tuple(std::move(from), std::move(to), weight);
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

  FeatureMappingModelFactory(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~FeatureMappingModelFactory() = default;

  virtual const std::string& get_type() const {
    return type_id_;
  }

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const;

private:
  std::shared_ptr<FeatureCache> cache_;
  std::string type_id_ = "mapping";
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_FEATURE_MAPPING_MODEL_H_ */
