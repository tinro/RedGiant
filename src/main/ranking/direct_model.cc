#include "direct_model.h"

#include <vector>
#include <utility>

#include "data/query_request.h"
#include "index/document_query.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<IntermQuery> DirectModel::process(const QueryRequest& request) const {
  if (request.is_debug()) {
    LOG_INFO(logger, "[query:%s] using direct model",
        request.get_request_id().c_str());
  }

  IntermQuery::QueryFeatures terms;
  for (const auto& fv: request.get_feature_vectors()) {
    for (const auto& f: fv.get_features()) {
      terms.emplace_back(f.first->get_id(), static_cast<IntermQuery::QueryWeight>(f.second));
      if (request.is_debug()) {
        LOG_TRACE(logger, "[query:%s] build feature 0x%016llx with weight %lf",
            request.get_request_id().c_str(), (unsigned long long)(f.first->get_id()), (double)(f.second));
      }
    }
  }
  return std::make_unique<IntermQuery>(std::move(terms));
}

std::unique_ptr<RankingModel> DirectModelFactory::create_model(const rapidjson::Value& config) const {
  std::string name;
  std::string type;
  if (json_try_get_value(config, "name", name) && json_try_get_value(config, "type", type)) {
    LOG_DEBUG(logger, "creating model %s in type %s", name.c_str(), type.c_str());
  }

  return std::make_unique<DirectModel>();
}


} /* namespace redgiant */
