#include <string>
#include <utility>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_model.h"
#include "data/query_request.h"
#include "ranking/direct_model.h"
#include "ranking/model_manager.h"
#include "utils/json_utils.h"
#include "utils/logger.h"

namespace redgiant {
class ModelManagerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ModelManagerTest);
  CPPUNIT_TEST(test_register);
  CPPUNIT_TEST(test_create);
  CPPUNIT_TEST(test_process);
  CPPUNIT_TEST_SUITE_END();

public:
  ModelManagerTest() = default;
  virtual ~ModelManagerTest() = default;

protected:
  void test_register() {
    auto mmf = create_model_manager_factory();
    int ret = mmf->register_model_factory(std::make_shared<DirectModelFactory>());
    CPPUNIT_ASSERT_EQUAL(0, ret);
    // duplicate
    ret = mmf->register_model_factory(std::make_shared<DirectModelFactory>());
    CPPUNIT_ASSERT_EQUAL(-1, ret);
  }

  void test_create() {
    auto mm = create_model_manager();
    CPPUNIT_ASSERT(!!mm);

    // type should match
    ModelManager * mmp = dynamic_cast<ModelManager*>(mm.get());
    CPPUNIT_ASSERT(!!mmp);
    // model should exist
    CPPUNIT_ASSERT(mmp->get_model("default_a"));
    // model should in type DefaultModel
    CPPUNIT_ASSERT(dynamic_cast<const DirectModel*>(mmp->get_model("default_a")));
    // model should exist
    CPPUNIT_ASSERT(mmp->get_model("mock_b"));
    // model should in type MockModel
    CPPUNIT_ASSERT(dynamic_cast<const MockModel*>(mmp->get_model("mock_b")));
    // model should exist
    CPPUNIT_ASSERT(mmp->get_model("mock_c"));
    // model should in type MockModel
    CPPUNIT_ASSERT(dynamic_cast<const MockModel*>(mmp->get_model("mock_c")));

    // not exist
    CPPUNIT_ASSERT(mmp->get_model("default") == nullptr);
    CPPUNIT_ASSERT(mmp->get_model("direct") == nullptr);
    CPPUNIT_ASSERT(mmp->get_model("") == nullptr);
  }

  void test_process() {
    auto mm = create_model_manager();
    CPPUNIT_ASSERT(!!mm);

    auto req1 = mock_request_1();
    auto iq1 = mm->process(*req1);
    CPPUNIT_ASSERT(!!iq1);

    auto req2 = mock_request_2();
    auto iq2 = mm->process(*req2);
    CPPUNIT_ASSERT(!!iq2);
  }

private:
  std::unique_ptr<ModelManagerFactory> create_model_manager_factory() {
    return std::unique_ptr<ModelManagerFactory>(new ModelManagerFactory());
  }

  std::unique_ptr<RankingModel> create_model_manager() {
    auto mmf = create_model_manager_factory();
    mmf->register_model_factory(std::make_shared<DirectModelFactory>());
    mmf->register_model_factory(std::make_shared<MockModelFactory>());

    char j[] = R"({
      "default_model": "default_a",
      "models": [
        { "name": "default_a", "type": "direct" },
        { "name": "mock_b", "type": "mock" },
        { "name": "mock_c", "type": "mock" }
      ]
    })";

    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    return mmf->create_model(conf);
  }

  std::unique_ptr<QueryRequest> mock_request_1() {
    std::unique_ptr<QueryRequest> req(new QueryRequest("0001", 10, "default_a", StopWatch(), true));
    return req;
  }

  std::unique_ptr<QueryRequest> mock_request_2() {
    std::unique_ptr<QueryRequest> req(new QueryRequest("0002", 10, "", StopWatch(), true));
    return req;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ModelManagerTest);
} /* namespace redgiant */
