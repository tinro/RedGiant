#include "ranking/feature_mapping_model.h"

#include <map>
#include <utility>
#include <vector>

#include "data/feature.h"
#include "data/feature_space_manager.h"
#include "data/feature_vector.h"
#include "data/query_request.h"
#include "index/document_query.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<IntermQuery> FeatureMappingModel::process(const QueryRequest& request) const {
  std::map<IntermQuery::FeatureId, IntermQuery::QueryWeight> terms;
  if (request.is_debug()) {
    LOG_INFO(logger, "[query:%s] using feature mapping model",
        request.get_request_id().c_str());
  }

  for (const FeatureVector& fv: request.get_feature_vectors()) {
    // find the mapped feature spaces. there may be multiple target spaces
    auto range = mappings_.equal_range(fv.get_space().get_id());
    if (range.first == mappings_.end()) {
      if (request.is_debug()) {
        LOG_WARN(logger, "[query:%s] feature space %s not found in model mappings!",
            request.get_request_id().c_str(), fv.get_space_name().c_str());
      }
      continue;
    }

    // for all mapped spaces
    for (auto iter = range.first; iter != range.second; ++iter) {
      Score weight = std::get<2>(iter->second);
      const auto& space = *std::get<1>(iter->second);
      if (weight <= 0) {
        if (request.is_debug()) {
          LOG_WARN(logger, "[query:%s] feature space %s weight %lf is not positive!",
              request.get_request_id().c_str(), fv.get_space_name().c_str(), weight);
        }
        continue;
      }

      // load features from request
      for (const FeatureVector::FeaturePair& feature_pair: fv.get_features()) {
        // project the feature to the mapped space
        Feature::FeatureId id = space.project_to_space(feature_pair.first->get_id());
        Score s = static_cast<Score>(feature_pair.second) * weight;
        // try insert the score if exists.
        auto insert_ret = terms.emplace(id, s);
        // or add the score to the existing scores
        if (!insert_ret.second) {
          insert_ret.first->second += s;
          if (request.is_debug()) {
            LOG_TRACE(logger, "[query:%s] build feature 0x%016llx from 0x%016llx, update weight to %lf",
                request.get_request_id().c_str(), (unsigned long long)id,
                (unsigned long long)(feature_pair.first->get_id()), insert_ret.first->second);
          }
        } else {
          if (request.is_debug()) {
            LOG_TRACE(logger, "[query:%s] build feature 0x%016llx from 0x%016llx, insert with weight %lf",
                request.get_request_id().c_str(), (unsigned long long)id,
                (unsigned long long)(feature_pair.first->get_id()), insert_ret.first->second);
          }
        } // terms.emplace
      } // for feature_pair
    } // for iter in range
  } // for fv
  return std::unique_ptr<IntermQuery>(new IntermQuery({terms.begin(), terms.end()}));
}

std::unique_ptr<RankingModel> FeatureMappingModelFactory::create_model(const rapidjson::Value& config) const {
  if (!config.IsObject()) {
     LOG_ERROR(logger, "failed to parse ranking json config.");
     return nullptr;
  }

  std::string name;
  std::string type;
  if (json_try_get_value(config, "name", name) && json_try_get_value(config, "type", type)) {
    LOG_DEBUG(logger, "creating model %s in type %s", name.c_str(), type.c_str());
  }

  auto mappings = json_get_array(config, "mappings");
  if (!mappings) {
    LOG_ERROR(logger, "models json config is missing!");
    return nullptr;
  }

  std::unique_ptr<FeatureMappingModel> model(new FeatureMappingModel());
  for (auto iter = mappings->Begin(); iter != mappings->End(); ++iter) {
    const auto& mapping = *iter;
    std::string from;
    std::string to;
    double weight;

    if (!json_try_get_value(mapping, "from", from)
        || !json_try_get_value(mapping, "to", to)
        || !json_try_get_value(mapping, "weight", weight)) {
      LOG_ERROR(logger, "unknown mapping config!");
      continue;
    }

    std::shared_ptr<FeatureSpace> from_space = feature_spaces_->get_space(from);
    if (!from_space) {
      LOG_ERROR(logger, "undefined feature space %s!", from.c_str());
      continue;
    }

    std::shared_ptr<FeatureSpace> to_space = feature_spaces_->get_space(to);
    if (!to_space) {
      LOG_ERROR(logger, "undefined feature space %s!", to.c_str());
      continue;
    }

    if (weight <= 0) {
      LOG_ERROR(logger, "feature mapping weight %f is not positive!", weight);
      continue;
    }

    LOG_TRACE(logger, "mapping feature space %s to %s with weight %lf",
        from_space->get_name().c_str(), to_space->get_name().c_str(), weight);
    model->set_mapping(std::move(from_space), std::move(to_space), weight);
  }
  return std::unique_ptr<RankingModel>(model.release());
}

} /* namespace redgiant */
