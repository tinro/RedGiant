#include <string>
#include <memory>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "data/feature_space_manager.h"
#include "third_party/rapidjson/document.h"

using namespace std;

namespace redgiant {
class FeatureSpaceManagerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FeatureSpaceManagerTest);
  CPPUNIT_TEST(test_create_feature_space);
  CPPUNIT_TEST(test_initialize_from_json);
  CPPUNIT_TEST_SUITE_END();

public:
  FeatureSpaceManagerTest() = default;
  virtual ~FeatureSpaceManagerTest() = default;

protected:
  void test_create_feature_space() {
    auto feature_spaces = std::unique_ptr<FeatureSpaceManager>(new FeatureSpaceManager());
    auto space_a = feature_spaces->create_space("A", 1, FeatureSpace::SpaceType::kInteger);
    auto space_b = feature_spaces->create_space("BB", 2, FeatureSpace::SpaceType::kString);
    auto space_c = feature_spaces->create_space("CCC", 3, FeatureSpace::SpaceType::kInteger);

    CPPUNIT_ASSERT(!!space_a); // not null
    CPPUNIT_ASSERT(!!space_b); // not null
    CPPUNIT_ASSERT(!!space_c); // not null

    auto aa = feature_spaces->get_space("A");
    CPPUNIT_ASSERT(space_a == aa);
    CPPUNIT_ASSERT_EQUAL(1ULL, (unsigned long long)(aa->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("A"), aa->get_name());

    auto bb = feature_spaces->get_space("BB");
    CPPUNIT_ASSERT(space_b == bb);
    CPPUNIT_ASSERT_EQUAL(2ULL, (unsigned long long)(bb->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("BB"), bb->get_name());

    // replace "A"
    auto space_d = feature_spaces->create_space("A", 4, FeatureSpace::SpaceType::kString);
    auto dd = feature_spaces->get_space("A");
    CPPUNIT_ASSERT(!!space_d); // not null
    CPPUNIT_ASSERT(space_d == dd);
    CPPUNIT_ASSERT_EQUAL(4ULL, (unsigned long long)(dd->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("A"), dd->get_name());
  }

  void test_initialize_from_json() {
    char j[] = R"([
      {"id": 1, "name": "category",  "type": "integer"},
      {"id": 2, "name": "entity",    "type": "string"},
      {"id": 3, "name": "publisher", "type": "string"}
    ])";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    auto feature_spaces = std::make_shared<FeatureSpaceManager>();
    int ret = feature_spaces->initialize(conf);

    CPPUNIT_ASSERT_EQUAL(0, ret);
    CPPUNIT_ASSERT(feature_spaces->get_space("category"));
    CPPUNIT_ASSERT(feature_spaces->get_space("entity"));
    CPPUNIT_ASSERT(feature_spaces->get_space("publisher"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FeatureSpaceManagerTest);
} /* namespace redgiant */
