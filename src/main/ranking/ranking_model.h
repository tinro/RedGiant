#ifndef SRC_MAIN_RANKING_RANKING_MODEL_H_
#define SRC_MAIN_RANKING_RANKING_MODEL_H_

#include <memory>

namespace redgiant {
class QueryBuilder;
class QueryRequest;
class RankingModelConfig;

class RankingModel {
public:
  RankingModel() = default;
  virtual ~RankingModel() = default;

  virtual std::unique_ptr<QueryBuilder> process(const QueryRequest& request) const = 0;
};

class RankingModelFactory {
public:
  RankingModelFactory() = default;
  virtual ~RankingModelFactory() = default;

  virtual const std::string& get_type() const = 0;

  virtual std::unique_ptr<RankingModel> create_model(const RankingModelConfig& config,
      const std::string& directory) const = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_RANKING_MODEL_H_ */
