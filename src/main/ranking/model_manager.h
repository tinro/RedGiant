#ifndef SRC_MAIN_RANKING_MODEL_MANAGER_H_
#define SRC_MAIN_RANKING_MODEL_MANAGER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ranking/ranking_model.h"
#include "third_party/rapidjson/document.h"

namespace redgiant {
class ModelManager: public RankingModel {
public:
  ModelManager() = default;
  virtual ~ModelManager() = default;

  virtual std::unique_ptr<IntermQuery> process(const QueryRequest& request) const;

  void set_model(std::string name, std::unique_ptr<RankingModel> model) {
    models_.emplace(std::move(name), std::move(model));
  }

  const RankingModel* get_model(const std::string& name) const {
    auto iter = models_.find(name);
    if (iter != models_.end()) {
      if (iter->second) {
        return iter->second.get();
      }
    }
    return nullptr;
  }

  void set_default_model_name(std::string name) {
    default_model_name_ = std::move(name);
  }

  const std::string& get_default_model_name() const {
    return default_model_name_;
  }

private:
  // mapping between model type and handler model factory
  std::unordered_map<std::string, std::unique_ptr<RankingModel>> models_;
  std::string default_model_name_;
};

class ModelManagerFactory: public RankingModelFactory {
public:
  ModelManagerFactory() = default;
  virtual ~ModelManagerFactory() = default;

  int register_model_factory(std::shared_ptr<RankingModelFactory> model_factory);

  virtual const std::string& get_type() const {
    return type_id_;
  }

  std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const;

  std::shared_ptr<RankingModelFactory> get_model_factory(const std::string& type) const {
    auto iter = model_factories_.find(type);
    if (iter != model_factories_.end()) {
      return iter->second;
    }
    return nullptr;
  }

private:
  // mapping between model type and handler model factory
  std::unordered_map<std::string, std::shared_ptr<RankingModelFactory>> model_factories_;
  std::string type_id_ = "model_manager";
};

} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_MODEL_MANAGER_H_ */
