#include "ranking/model_manager.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <utility>

#include "data/query_request.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<IntermQuery> ModelManager::process(const QueryRequest& request) const {
  // dispatch the context
  const std::string* pname = &(request.get_model_name());
  if (pname->empty()) {
    pname = &default_model_name_;
    if (request.is_debug()) {
      LOG_INFO(logger, "[query:%s] model not set, use default model %s", request.get_request_id().c_str(),
          pname->c_str());
    }
  }

  auto iter = models_.find(*pname);
  if (iter == models_.end() || !(iter->second)) {
    LOG_ERROR(logger, "[query:%s] model %s not found.", request.get_request_id().c_str(), pname->c_str());
    return nullptr;
  }

  LOG_DEBUG(logger, "[query:%s] use ranking model %s.", request.get_request_id().c_str(), pname->c_str());
  // load the model
  return iter->second->process(request);
}

int ModelManagerFactory::register_model_factory(std::shared_ptr<RankingModelFactory> factory) {
  const std::string& type = factory->get_type();
  LOG_INFO(logger, "registered model type %s.", type.c_str());
  auto ret = model_factories_.emplace(type, std::move(factory));
  if (!ret.second) {
    LOG_ERROR(logger, "duplicate model type %s", type.c_str());
    return -1;
  }
  return 0;
}

std::unique_ptr<RankingModel> ModelManagerFactory::create_model(const rapidjson::Value& config) const {
  if (!config.IsObject()) {
    LOG_ERROR(logger, "failed to parse ranking json config.");
    return nullptr;
  }

  auto models = json_get_array(config, "models");
  if (!models) {
    LOG_ERROR(logger, "models json config is missing!");
    return nullptr;
  }

  auto mm = std::make_unique<ModelManager>();
  const char* default_model = json_get_str(config, "default_model");
  if (default_model && default_model[0]) {
    mm->set_default_model_name(default_model);
  }

  for (auto iter = models->Begin(); iter != models->End(); ++iter) {
    const auto& model = *iter;
    std::string type;
    std::string name;

    if (!json_try_get_value(model, "type", type)) {
      LOG_ERROR(logger, "model config error: type is required!");
      continue;
    }
    auto factory_iter = model_factories_.find(type);
    if (factory_iter == model_factories_.end()) {
      LOG_WARN(logger, "ranking model type %s is unknown!", type.c_str());
      continue;
    }

    if (!json_try_get_value(model, "name", name)) {
      LOG_ERROR(logger, "model config error: name is required!");
      continue;
    }

    auto model_ptr = factory_iter->second->create_model(model);
    mm->set_model(std::move(name), std::move(model_ptr));
  }

  return std::unique_ptr<RankingModel>(mm.release());
}

} /* namespace redgiant */
