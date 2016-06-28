#include "data/feature_space.h"

#include <string>
#include <memory>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "data/feature.h"

using namespace std;

namespace redgiant {
class FeatureSpaceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FeatureSpaceTest);
  CPPUNIT_TEST(test_string_space);
  CPPUNIT_TEST(test_integer_space);
  CPPUNIT_TEST(test_feature_id);
  CPPUNIT_TEST_SUITE_END();

public:
  FeatureSpaceTest() = default;
  virtual ~FeatureSpaceTest() = default;

protected:
  void test_string_space() {
    auto space = std::make_unique<FeatureSpace>("AAA", 1, FeatureSpace::SpaceType::kString);
    CPPUNIT_ASSERT_EQUAL(string("AAA"), space->get_name());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)space->get_id());
    CPPUNIT_ASSERT_EQUAL(string("string"), space->get_type_name());

    auto f1 = space->create_feature("abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), f1->get_key());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)(FeatureSpace::get_part_space_id(f1->get_id())));

    auto f2 = space->create_feature("123");
    CPPUNIT_ASSERT_EQUAL(string("123"), f2->get_key());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)(FeatureSpace::get_part_space_id(f2->get_id())));
  }

  void test_integer_space() {
    auto space = std::make_unique<FeatureSpace>("BBB", 2, FeatureSpace::SpaceType::kInteger);
    CPPUNIT_ASSERT_EQUAL(string("BBB"), space->get_name());
    CPPUNIT_ASSERT_EQUAL(2UL, (unsigned long)space->get_id());
    CPPUNIT_ASSERT_EQUAL(string("integer"), space->get_type_name());

    auto f1 = space->create_feature("abc");
    CPPUNIT_ASSERT(!f1); // null

    auto f2 = space->create_feature("123");
    CPPUNIT_ASSERT_EQUAL(string("123"), f2->get_key());
    CPPUNIT_ASSERT_EQUAL(2UL, (unsigned long)(FeatureSpace::get_part_space_id(f2->get_id())));
    CPPUNIT_ASSERT_EQUAL(123ULL, (unsigned long long)(FeatureSpace::get_part_feature_id(f2->get_id())));

    auto f3 = space->create_feature("4.5");
    CPPUNIT_ASSERT_EQUAL(string("4.5"), f3->get_key());
    CPPUNIT_ASSERT_EQUAL(2UL, (unsigned long)(FeatureSpace::get_part_space_id(f3->get_id())));
    CPPUNIT_ASSERT_EQUAL(4ULL, (unsigned long long)(FeatureSpace::get_part_feature_id(f3->get_id())));
  }

  void test_feature_id() {
    auto s1 = std::make_unique<FeatureSpace>("AAA", 1, FeatureSpace::SpaceType::kString);
    auto s2 = std::make_unique<FeatureSpace>("BBB", 2, FeatureSpace::SpaceType::kInteger);

    auto f1 = s1->create_feature("abc");
    Feature::FeatureId id2 = s2->project_to_space(f1->get_id());

    // the ids share the same feature_id part
    CPPUNIT_ASSERT(FeatureSpace::get_part_feature_id(f1->get_id()) == FeatureSpace::get_part_feature_id(id2));
    // the second id is in the new space
    CPPUNIT_ASSERT_EQUAL(2UL, (unsigned long)(FeatureSpace::get_part_space_id(id2)));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FeatureSpaceTest);
} /* namespace redgiant */
