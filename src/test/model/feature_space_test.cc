#include <string>
#include <memory>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "model/feature.h"
#include "model/feature_space.h"

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
    std::unique_ptr<FeatureSpace> space(new FeatureSpace("AAA", 1, FeatureSpace::kString));
    CPPUNIT_ASSERT_EQUAL(string("AAA"), space->get_name());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)space->get_id());
    CPPUNIT_ASSERT_EQUAL(FeatureSpace::kString, space->get_type());

    auto f1 = space->create_feature("abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), f1->get_key());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)(FeatureSpace::get_part_space_id(f1->get_id())));

    auto f2 = space->create_feature("123");
    CPPUNIT_ASSERT_EQUAL(string("123"), f2->get_key());
    CPPUNIT_ASSERT_EQUAL(1UL, (unsigned long)(FeatureSpace::get_part_space_id(f2->get_id())));
  }

  void test_integer_space() {
    std::unique_ptr<FeatureSpace> space(new FeatureSpace("BBB", 2, FeatureSpace::kInteger));
    CPPUNIT_ASSERT_EQUAL(string("BBB"), space->get_name());
    CPPUNIT_ASSERT_EQUAL(2UL, (unsigned long)space->get_id());
    CPPUNIT_ASSERT_EQUAL(FeatureSpace::kInteger, space->get_type());

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
    std::unique_ptr<FeatureSpace> s1(new FeatureSpace("AAA", 1, FeatureSpace::kString));
    std::unique_ptr<FeatureSpace> s2(new FeatureSpace("BBB", 2, FeatureSpace::kInteger));

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
