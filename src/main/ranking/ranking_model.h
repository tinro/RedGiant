#ifndef SRC_MAIN_RANKING_RANKING_MODEL_H_
#define SRC_MAIN_RANKING_RANKING_MODEL_H_

#include <memory>

#include "third_party/rapidjson/document.h"

namespace redgiant {
class DocumentQuery;
class QueryRequest;

class RankingModel {
public:
  RankingModel() = default;
  virtual ~RankingModel() = default;

  virtual std::unique_ptr<DocumentQuery> process(const QueryRequest& request) const = 0;
};

class RankingModelConfig {
  RankingModelConfig() = default;
  virtual ~RankingModelConfig() = default;
};

class RankingModelFactory {
public:
  RankingModelFactory() = default;
  virtual ~RankingModelFactory() = default;

  virtual const std::string& get_type() const = 0;

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_RANKING_MODEL_H_ */
