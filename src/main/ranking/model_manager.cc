#include "ranking/model_manager.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <memory>
#include <utility>
#include "data/query_request.h"
#include "index/document_query.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<DocumentQuery> ModelManager::process(const QueryRequest& request) const {
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

  // load the model
  return iter->second->process(request);
}

int ModelManagerFactory::register_model_factory(std::shared_ptr<RankingModelFactory> factory) {
  std::string type = factory->get_type();
  LOG_INFO(logger, "registered model type %s.", type.c_str());
  auto ret = model_factories_.emplace(std::move(type), std::move(factory));

  if (!ret.second) {
    LOG_ERROR(logger, "duplicate model type %s", factory->get_type().c_str());
    return -1;
  }
  return 0;
}

std::unique_ptr<RankingModel> ModelManagerFactory::create_model(const rapidjson::Value& config) const {
  if (!config.IsObject()) {
    LOG_ERROR(logger, "failed to parse ranking json config.");
    return nullptr;
  }

  if (!config.HasMember("models") || !config["models"].IsArray()) {
    LOG_ERROR(logger, "models json config is missing!");
    return nullptr;
  }

  std::unique_ptr<ModelManager> mm(new ModelManager());
  if (config.HasMember("default_model") && config["default_model"].IsString()) {
    std::string default_model = config["default_model"].GetString();
    mm->set_default_model_name(std::move(default_model));
  }

  const auto& models = config["models"];
  for (auto iter = models.Begin(); iter != models.End(); ++iter) {
    const auto& model = *iter;
    if (!model.IsObject()) {
      LOG_ERROR(logger, "model config error!");
      continue;
    }

    if (!model.HasMember("type") || !model["type"].IsString()) {
      LOG_ERROR(logger, "model config error: type is required!");
      continue;
    }
    std::string type = model["type"].GetString();
    auto factory_iter = model_factories_.find(type);
    if (factory_iter == model_factories_.end()) {
      LOG_WARN(logger, "ranking model type %s is unknown!", type.c_str());
      continue;
    }

    if (!model.HasMember("name") || !model["name"].IsString()) {
      LOG_ERROR(logger, "model config error: name is required!");
      continue;
    }

    std::string name = model["name"].GetString();
    auto model_ptr = factory_iter->second->create_model(model);
    if (!model_ptr) {
      LOG_ERROR(logger, "create model %s failed!", name.c_str());
      continue;
    }
    mm->set_model(std::move(name), std::move(model_ptr));
  }

  return std::unique_ptr<RankingModel>(mm.release());
}

} /* namespace redgiant */
