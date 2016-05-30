#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "query/simple_query_executor.h"

#include "data/query_request.h"
#include "data/query_result.h"
#include "data/feature_cache.h"
#include "data/feature_space.h"
#include "data/feature_vector.h"
#include "index/document_index_manager.h"
#include "ranking/direct_model.h"
#include "utils/logger.h"

using namespace std;

namespace redgiant {
class SimpleQueryExecutorTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SimpleQueryExecutorTest);
  CPPUNIT_TEST(test_create);
  CPPUNIT_TEST(test_execute_1);
  CPPUNIT_TEST(test_execute_2);
  CPPUNIT_TEST(test_execute_3);
  CPPUNIT_TEST_SUITE_END();

public:
  SimpleQueryExecutorTest() = default;
  virtual ~SimpleQueryExecutorTest() = default;

protected:
  void test_create() {
    auto cache = create_feature_cache();
    auto model = create_model();
    auto index = create_index(*cache);
    auto executor = create_executor(index.get(), model.get());

    CPPUNIT_ASSERT(!!executor);
    CPPUNIT_ASSERT(!!dynamic_cast<SimpleQueryExecutor*>(executor.get()));
  }

  void test_execute_1() {
    auto cache = create_feature_cache();
    auto model = create_model();
    auto index = create_index(*cache);
    auto executor = create_executor(index.get(), model.get());

    auto request = create_request_1(*cache);
    auto result = executor->execute(*request);

    CPPUNIT_ASSERT(!!result);

    auto ids = result->get_results();
    CPPUNIT_ASSERT_EQUAL(4, (int)ids.size());

    CPPUNIT_ASSERT_EQUAL(string("00000000-0001-0000-0000-000000000000"), ids[0].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.7, ids[0].second, 0.00001);

    CPPUNIT_ASSERT_EQUAL(string("00000000-0002-0000-0000-000000000000"), ids[1].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, ids[1].second, 0.00001);

    CPPUNIT_ASSERT_EQUAL(string("00000000-0005-0000-0000-000000000000"), ids[2].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, ids[2].second, 0.00001);

    CPPUNIT_ASSERT_EQUAL(string("00000000-0003-0000-0000-000000000000"), ids[3].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.4, ids[3].second, 0.00001);
  }

  void test_execute_2() {
    auto cache = create_feature_cache();
    auto model = create_model();
    auto index = create_index(*cache);
    auto executor = create_executor(index.get(), model.get());

    auto request = create_request_2(*cache);
    auto result = executor->execute(*request);

    CPPUNIT_ASSERT(!!result);

    auto ids = result->get_results();
    CPPUNIT_ASSERT_EQUAL(2, (int)ids.size());

    CPPUNIT_ASSERT_EQUAL(string("00000000-0001-0000-0000-000000000000"), ids[0].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.6, ids[0].second, 0.00001);

    CPPUNIT_ASSERT_EQUAL(string("00000000-0002-0000-0000-000000000000"), ids[1].first);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, ids[1].second, 0.00001);
  }

  void test_execute_3() {
    auto cache = create_feature_cache();
    auto model = create_model();
    auto index = create_index(*cache);
    auto executor = create_executor(index.get(), model.get());

    auto request = create_request_3(*cache);
    auto result = executor->execute(*request);

    CPPUNIT_ASSERT(!!result);

    auto ids = result->get_results();
    CPPUNIT_ASSERT_EQUAL(0, (int)ids.size());
  }

private:
  std::shared_ptr<FeatureCache> create_feature_cache() {
    char j[] = R"([
      {"id": 1, "name": "category",           "type": "integer"},
      {"id": 2, "name": "entity",             "type": "string"},
      {"id": 3, "name": "publisher",          "type": "string"}
    ])";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    auto fc = std::make_shared<FeatureCache>();
    fc->initialize(conf);
    return fc;
  }

  std::unique_ptr<RankingModel> create_model() {
    auto mmf = std::make_shared<DirectModelFactory>();
    char j[] = R"({ "name": "default", "type": "direct" })";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    return mmf->create_model(conf);
  }

  std::shared_ptr<Document> create_document(std::string id,
      std::vector<std::pair<std::shared_ptr<FeatureSpace>, std::vector<std::pair<std::string, double>>>> features) {
    auto doc = std::make_shared<Document>(std::move(id));
    for (auto& s: features) {
      FeatureVector vec(std::move(s.first));
      for (auto& p: s.second) {
        vec.add_feature(p.first, p.second);
      }
      doc->add_feature_vector(std::move(vec));
    }
    return doc;
  }

  std::unique_ptr<DocumentIndexManager> create_index(FeatureCache& cache) {
    auto index = std::unique_ptr<DocumentIndexManager>(new DocumentIndexManager(1000, 1000));
    auto space_cat = cache.get_space("category");
    auto space_ent = cache.get_space("entity");
    // create document vectors
    index->update(create_document(
        "00000000-0001-0000-0000-000000000000",
        {
          { space_cat, {{"1", 1.1}, {"2", 2.2}, {"3", 3.3}}},
          { space_ent, {{"AA", 0.1}, {"BB", 0.2}}},
        }), 1);

    index->update(create_document(
        "00000000-0003-0000-0000-000000000000",
        {
          { space_cat, {{"1", 1.5}, {"3", 0.2}}},
          { space_ent, {{"AA", 1.0}}},
        }), 1);

    index->update(create_document(
        "00000000-0004-0000-0000-000000000000",
        {
          { space_cat, {{"1001", 1.0}, {"1002", 1.5}}},
          { space_ent, {{"xxx", 0.5}, {"yyy", 0.5}}},
        }), 1);

    index->update(create_document(
        "00000000-0005-0000-0000-000000000000",
        {
          { space_cat, {{"1002", 2.0}, {"1003", 0.3}}},
          { space_ent, {{"xxx", 0.5}, {"zzz", 0.5}}},
        }), 1);

    index->update(create_document(
        "00000000-0002-0000-0000-000000000000",
        {
          { space_cat, {{"3", 1.5}, {"2", 1.4}}},
          { space_ent, {{"BB", 0.3}, {"CC", 1.0}}},
        }), 1);

    index->do_maintain(0);
    return index;
  }

  std::unique_ptr<QueryExecutor> create_executor(DocumentIndexManager* index, RankingModel* model) {
    auto factory = std::make_shared<SimpleQueryExecutorFactory>(index, model);
    return factory->create_executor();
  }

  std::shared_ptr<QueryRequest> create_request_1(FeatureCache& cache) {
    auto request = std::make_shared<QueryRequest>("0001", 50, "", StopWatch(), true);
    auto space_cat = cache.get_space("category");
    auto space_ent = cache.get_space("entity");
    request->add_feature_vector(FeatureVector(space_cat, {
        {"3", 2.0}
    }));
    request->add_feature_vector(FeatureVector(space_ent, {
        {"AA", 1.0},
        {"zzz", 5.0}
    }));
    return request;
  }

  std::shared_ptr<QueryRequest> create_request_2(FeatureCache& cache) {
    auto request = std::make_shared<QueryRequest>("0002", 2, "", StopWatch(), true);
    auto space_cat = cache.get_space("category");
    request->add_feature_vector(FeatureVector(space_cat, {
        {"3", 2.0}
    }));
    return request;
  }

  std::shared_ptr<QueryRequest> create_request_3(FeatureCache& cache) {
    auto request = std::make_shared<QueryRequest>("0003", 50, "", StopWatch(), true);
    auto space_cat = cache.get_space("category");
    auto space_ent = cache.get_space("entity");
    request->add_feature_vector(FeatureVector(space_cat, {
        {"9999", 2.0}
    }));
    request->add_feature_vector(FeatureVector(space_ent, {
        {"not_exist", 1.0}
    }));
    return request;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SimpleQueryExecutorTest);
} /* namespace redgiant */
