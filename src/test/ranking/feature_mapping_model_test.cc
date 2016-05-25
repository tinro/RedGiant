#include <algorithm>
#include <string>
#include <utility>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_model.h"
#include "data/feature_cache.h"
#include "data/feature_vector.h"
#include "data/query_request.h"
#include "ranking/feature_mapping_model.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {
class FeatureMappingModelTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FeatureMappingModelTest);
  CPPUNIT_TEST(test_create);
  CPPUNIT_TEST(test_process);
  CPPUNIT_TEST_SUITE_END();

public:
  FeatureMappingModelTest() = default;
  virtual ~FeatureMappingModelTest() = default;

protected:
  void test_create() {
    auto fc = create_feature_cache();
    auto mm = create_model(fc);
    CPPUNIT_ASSERT(!!mm);
    CPPUNIT_ASSERT(dynamic_cast<FeatureMappingModel*>(mm.get()));
  }

  void test_process() {
    auto fc = create_feature_cache();
    auto mm = create_model(fc);
    CPPUNIT_ASSERT(!!mm);

    auto req = mock_request_1(*fc);
    auto iq = mm->process(*req);
    CPPUNIT_ASSERT(!!iq);

    const auto& features = iq->get_features();
    CPPUNIT_ASSERT_EQUAL(4, (int)features.size());

    auto iter = find_interm_query_pair(features, *fc, "category", "123");
    CPPUNIT_ASSERT(iter != features.end());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, iter->second, 0.00001);

    iter = find_interm_query_pair(features, *fc, "category", "456");
    CPPUNIT_ASSERT(iter != features.end());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, iter->second, 0.00001);

    iter = find_interm_query_pair(features, *fc, "category", "789");
    CPPUNIT_ASSERT(iter != features.end());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, iter->second, 0.00001);

    iter = find_interm_query_pair(features, *fc, "category", "1000");
    CPPUNIT_ASSERT(iter != features.end());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, iter->second, 0.00001);
  }

private:
  std::shared_ptr<FeatureCache> create_feature_cache() {
    char j[] = R"([
      {"id": 1, "name": "category",           "type": "integer"},
      {"id": 2, "name": "entity",             "type": "string"},
      {"id": 3, "name": "publisher",          "type": "string"},
      {"id": 4, "name": "category_inferred",  "type": "integer"},
      {"id": 5, "name": "category_declared",  "type": "integer"}
    ])";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    auto fc = std::make_shared<FeatureCache>();
    fc->initialize(conf);
    return std::move(fc);
  }

  std::unique_ptr<RankingModel> create_model(std::shared_ptr<FeatureCache> fc) {
    auto mmf = std::make_shared<FeatureMappingModelFactory>(std::move(fc));
    char j[] = R"({ "name": "category_only", "type": "mapping", "mappings": [
      { "from": "category_inferred", "to": "category", "weight": 2.0 },
      { "from": "category_declared", "to": "category", "weight": 3.0 }
    ]})";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);
    return mmf->create_model(conf);
  }

  std::unique_ptr<QueryRequest> mock_request_1(FeatureCache& fc) {
    std::unique_ptr<QueryRequest> req(new QueryRequest("0001", 10, "default_a", StopWatch(), true));
    req->add_feature_vector(FeatureVector(fc.get_space("category_inferred"), {
        {"123", 1.0},
        {"456", 2.0},
        {"789", 3.0}
    }));
    req->add_feature_vector(FeatureVector(fc.get_space("category_declared"), {
        {"456", 3.0},
        {"1000", 1.0}
    }));
    req->add_feature_vector(FeatureVector(fc.get_space("entity"), {
        {"abc", 1.0},
        {"def", 2.0},
        {"ghi", 3.0}
    }));
    req->add_feature_vector(FeatureVector(fc.get_space("publisher"), {
        {"xyz", 1.0}
    }));
    return req;
  }

  auto find_interm_query_pair(const IntermQuery::QueryFeatures& features,
      const FeatureCache& fc, const std::string& space_name, const std::string& feature_key)
  -> IntermQuery::QueryFeatures::const_iterator {
    IntermQuery::FeatureId id = fc.get_space(space_name)->calculate_feature_id(feature_key);
    return find_if(features.begin(), features.end(),
        [id]
        (const std::pair<IntermQuery::FeatureId, IntermQuery::QueryWeight>& f) {
          return f.first == id;
        }
    );
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FeatureMappingModelTest);
} /* namespace redgiant */
