#include "data/feature_cache.h"

#include <string>
#include <memory>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "third_party/rapidjson/document.h"

using namespace std;

namespace redgiant {
class FeatureCacheTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FeatureCacheTest);
  CPPUNIT_TEST(test_create_feature_space);
  CPPUNIT_TEST(test_create_feature);
  CPPUNIT_TEST(test_get_feature);
  CPPUNIT_TEST(test_initialize_from_json);
  CPPUNIT_TEST_SUITE_END();

public:
  FeatureCacheTest() = default;
  virtual ~FeatureCacheTest() = default;

protected:
  void test_create_feature_space() {
    auto cache = std::unique_ptr<FeatureCache>(new FeatureCache());
    auto space_a = cache->create_space("A", 1, FeatureSpace::SpaceType::kInteger);
    auto space_b = cache->create_space("BB", 2, FeatureSpace::SpaceType::kString);
    auto space_c = cache->create_space("CCC", 3, FeatureSpace::SpaceType::kInteger);

    CPPUNIT_ASSERT(!!space_a); // not null
    CPPUNIT_ASSERT(!!space_b); // not null
    CPPUNIT_ASSERT(!!space_c); // not null

    auto aa = cache->get_space("A");
    CPPUNIT_ASSERT(space_a == aa);
    CPPUNIT_ASSERT_EQUAL(1ULL, (unsigned long long)(aa->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("A"), aa->get_name());

    auto bb = cache->get_space("BB");
    CPPUNIT_ASSERT(space_b == bb);
    CPPUNIT_ASSERT_EQUAL(2ULL, (unsigned long long)(bb->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("BB"), bb->get_name());

    // replace "A"
    auto space_d = cache->create_space("A", 4, FeatureSpace::SpaceType::kString);
    auto dd = cache->get_space("A");
    CPPUNIT_ASSERT(!!space_d); // not null
    CPPUNIT_ASSERT(space_d == dd);
    CPPUNIT_ASSERT_EQUAL(4ULL, (unsigned long long)(dd->get_id()));
    CPPUNIT_ASSERT_EQUAL(string("A"), dd->get_name());
  }

  void test_create_feature() {
    auto cache = std::unique_ptr<FeatureCache>(new FeatureCache());
    auto space_a = cache->create_space("A", 1, FeatureSpace::SpaceType::kInteger);
    auto space_b = cache->create_space("BB", 2, FeatureSpace::SpaceType::kString);
    auto space_c = cache->create_space("CCC", 3, FeatureSpace::SpaceType::kInteger);

    auto f1 = cache->create_or_get_feature("111", "A");
    auto f2 = cache->create_or_get_feature("xxx", space_b);
    auto f3 = cache->create_or_get_feature("yyy", space_c);

    CPPUNIT_ASSERT(!!f1); // not null
    CPPUNIT_ASSERT_EQUAL(string("111"), f1->get_key());
    CPPUNIT_ASSERT_EQUAL(111ULL, (unsigned long long)FeatureSpace::get_part_feature_id(f1->get_id()));

    CPPUNIT_ASSERT(!!f2); // not null
    CPPUNIT_ASSERT_EQUAL(string("xxx"), f2->get_key());
    CPPUNIT_ASSERT(f2->get_id() != FeatureSpace::kInvalidId);

    CPPUNIT_ASSERT(!f3); // null
  }

  void test_get_feature() {
    auto cache = std::unique_ptr<FeatureCache>(new FeatureCache());
    auto space_a = cache->create_space("A", 1, FeatureSpace::SpaceType::kInteger);
    auto space_b = cache->create_space("BB", 2, FeatureSpace::SpaceType::kString);
    auto space_c = cache->create_space("CCC", 3, FeatureSpace::SpaceType::kInteger);

    auto f1 = cache->create_or_get_feature("111", "A");
    auto f2 = cache->create_or_get_feature("xxx", space_b);
    auto f3 = cache->create_or_get_feature("222", space_c);
    auto f4 = cache->create_or_get_feature("222", "CCC");

    CPPUNIT_ASSERT(f3 == f4); // null
    CPPUNIT_ASSERT_EQUAL(string("222"), f4->get_key());
  }

  void test_initialize_from_json() {
    char j[] = R"([
      {"id": 1, "name": "category",           "type": "integer"},
      {"id": 2, "name": "entity",             "type": "string"},
      {"id": 3, "name": "publisher",          "type": "string"}
    ])";
    rapidjson::MemoryStream ms(j, sizeof(j)/sizeof(j[0]));
    rapidjson::Document conf;
    conf.ParseStream(ms);

    auto cache = std::make_shared<FeatureCache>();
    int ret = cache->initialize(conf);

    CPPUNIT_ASSERT_EQUAL(0, ret);
    CPPUNIT_ASSERT(cache->get_space("category"));
    CPPUNIT_ASSERT(cache->get_space("entity"));
    CPPUNIT_ASSERT(cache->get_space("publisher"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FeatureCacheTest);
} /* namespace redgiant */
