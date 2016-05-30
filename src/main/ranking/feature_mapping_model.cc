#include "ranking/feature_mapping_model.h"

#include <map>
#include <utility>
#include <vector>

#include "data/feature.h"
#include "data/feature_space.h"
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

  for (const auto& fv: request.get_feature_vectors()) {
    // find if the input feature vector exists in mappings.
    auto iter = mappings_.find(fv.get_space_name());
    if (iter == mappings_.end()) {
      if (request.is_debug()) {
        LOG_WARN(logger, "[query:%s] feature space %s not found in model mappings!",
            request.get_request_id().c_str(), fv.get_space_name().c_str());
      }
      continue;
    }

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
    for (const auto& f: fv.get_features()) {
      // project the feature to the mapped space
      Feature::FeatureId id = space.project_to_space(f.first->get_id());
      Score s = static_cast<Score>(f.second) * weight;
      // try insert the score if exists.
      auto insert_ret = terms.emplace(id, s);
      // or add the score to the existing scores
      if (!insert_ret.second) {
        insert_ret.first->second += s;
        if (request.is_debug()) {
          LOG_TRACE(logger, "[query:%s] build feature 0x%016llx from 0x%016llx, update weight to %lf",
              request.get_request_id().c_str(), (unsigned long long)id,
              (unsigned long long)(f.first->get_id()), insert_ret.first->second);
        }
      } else {
        if (request.is_debug()) {
          LOG_TRACE(logger, "[query:%s] build feature 0x%016llx from 0x%016llx, insert with weight %lf",
              request.get_request_id().c_str(), (unsigned long long)id,
              (unsigned long long)(f.first->get_id()), insert_ret.first->second);
        }
      }
    }
  }
  return std::unique_ptr<IntermQuery>(new IntermQuery({terms.begin(), terms.end()}));
}

std::unique_ptr<RankingModel> FeatureMappingModelFactory::create_model(const rapidjson::Value& config) const {
  if (!config.IsObject()) {
     LOG_ERROR(logger, "failed to parse ranking json config.");
     return nullptr;
  }

  std::string name;
  std::string type;
  if (json_try_get_string(config, "name", name) && json_try_get_string(config, "type", type)) {
    LOG_DEBUG(logger, "creating model %s in type %s", name.c_str(), type.c_str());
  }

  auto mappings = json_try_get_array(config, "mappings");
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

    if (!json_try_get_string(mapping, "from", from)
        || !json_try_get_string(mapping, "to", to)
        || !json_try_get_double(mapping, "weight", weight)) {
      LOG_ERROR(logger, "unknown mapping config!");
      continue;
    }

    auto from_space = cache_->get_space(from);
    if (!from_space) {
      LOG_ERROR(logger, "undefined feature space %s!", from.c_str());
      continue;
    }

    auto to_space = cache_->get_space(to);
    if (!to_space) {
      LOG_ERROR(logger, "undefined feature space %s!", to.c_str());
      continue;
    }

    if (weight <= 0) {
      LOG_ERROR(logger, "feature mapping weight %f is not positive!", weight);
      continue;
    }

    LOG_DEBUG(logger, "mapping feature space %s to %s with weight %lf",
        from_space->get_name().c_str(), to_space->get_name().c_str(), weight);
    model->set_mapping(std::move(from_space), std::move(to_space), weight);
  }
  return std::unique_ptr<RankingModel>(model.release());
}

} /* namespace redgiant */
