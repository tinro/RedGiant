#ifndef SRC_MAIN_RANKING_DEFAULT_MODEL_H_
#define SRC_MAIN_RANKING_DEFAULT_MODEL_H_

#include <memory>
#include <string>
#include <utility>

#include "ranking/ranking_model.h"

namespace redgiant {
class DocumentQuery;
class QueryRequest;

/*
 * The default model directly picks all features from the query request
 *  and query them in the index.
 */
class DefaultModel: public RankingModel {
public:
  DefaultModel() = default;
  virtual ~DefaultModel() = default;

  virtual std::unique_ptr<DocumentQuery> process(const QueryRequest& request) const;
};

class DefaultModelFactory: public RankingModelFactory {
public:
  DefaultModelFactory() = default;
  virtual ~DefaultModelFactory() = default;

  virtual const std::string& get_type() const {
    return type_id_;
  }

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const {
    (void)config;
    return std::unique_ptr<RankingModel>(new DefaultModel());
  }

private:
  std::string type_id_ = "default";
};
} /* namespace redgiant */

#endif /* SRC_MAIN_RANKING_DEFAULT_MODEL_H_ */
