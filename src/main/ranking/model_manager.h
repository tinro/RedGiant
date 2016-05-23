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
  friend class RankingManagerTest;

  ModelManager() = default;
  virtual ~ModelManager() = default;

  virtual std::unique_ptr<DocumentQuery> process(const QueryRequest& request) const;

  void set_model(std::string name, std::unique_ptr<RankingModel> model) {
    models_.emplace(std::move(name), std::move(model));
  }

  void set_default_model_name(std::string name) {
    default_model_name_ = std::move(name);
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

  virtual std::string get_type() const {
    return "model_manager";
  }

  std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const;

private:
  // mapping between model type and handler model factory
  std::unordered_map<std::string, std::shared_ptr<RankingModelFactory>> model_factories_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_MODEL_MANAGER_H_ */
