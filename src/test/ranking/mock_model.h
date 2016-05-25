#ifndef SRC_TEST_RANKING_MOCK_MODEL_H_
#define SRC_TEST_RANKING_MOCK_MODEL_H_

#include <memory>
#include <string>
#include <utility>

#include "ranking/ranking_model.h"

namespace redgiant {
class MockModel: public RankingModel {
public:
  MockModel() = default;
  virtual ~MockModel() = default;

  virtual std::unique_ptr<IntermQuery> process(const QueryRequest& request) const {
    return nullptr;
  }
};

class MockModelFactory: public RankingModelFactory {
public:
  MockModelFactory() = default;
  virtual ~MockModelFactory() = default;

  virtual const std::string& get_type() const {
    return type_id_;
  }

  virtual std::unique_ptr<RankingModel> create_model(const rapidjson::Value& config) const {
    (void)config;
    return std::unique_ptr<RankingModel>(new MockModel());
  }

private:
  std::string type_id_ = "mock";
};
} /* namespace redgiant */

#endif /* SRC_TEST_RANKING_MOCK_MODEL_H_ */
