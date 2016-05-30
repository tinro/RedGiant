#ifndef SRC_MAIN_RANKING_DIRECT_MODEL_H_
#define SRC_MAIN_RANKING_DIRECT_MODEL_H_

#include <memory>
#include <string>
#include <utility>

#include "ranking/ranking_model.h"

namespace redgiant {
/*
 * The direct mapping model picks all features from the query request and query them in the index.
 */
class DirectModel: public RankingModel {
public:
  DirectModel() = default;
  virtual ~DirectModel() = default;

  virtual std::unique_ptr<IntermQuery> process(const QueryRequest& request) const;
};

class DirectModelFactory: public RankingModelFactory {
public:
  DirectModelFactory() = default;
  virtual ~DirectModelFactory() = default;

  virtual const std::string& get_type() const {
    return type_id_;
  }

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const;

private:
  std::string type_id_ = "direct";
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_DIRECT_MODEL_H_ */
