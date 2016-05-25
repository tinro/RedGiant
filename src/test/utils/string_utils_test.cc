#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "utils/string_utils.h"
#include "utils/logger.h"

using namespace std;

namespace redgiant {

class StringUtilsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StringUtilsTest);
  CPPUNIT_TEST(test_string_split);
  CPPUNIT_TEST(test_string_strip);
  CPPUNIT_TEST_SUITE_END();

public:

protected:
  void test_string_split() {
    auto output = string_split("test1:test2", ':');
    CPPUNIT_ASSERT_EQUAL(2, (int)output.size());
    CPPUNIT_ASSERT_EQUAL(string("test1"), output[0]);
    CPPUNIT_ASSERT_EQUAL(string("test2"), output[1]);

    auto output2 = string_split(",", ',');
    CPPUNIT_ASSERT_EQUAL(2, (int)output2.size());
    CPPUNIT_ASSERT_EQUAL(string(""), output2[0]);
    CPPUNIT_ASSERT_EQUAL(string(""), output2[1]);

    auto output3 = string_split("a,", ',');
    CPPUNIT_ASSERT_EQUAL(2, (int)output3.size());
    CPPUNIT_ASSERT_EQUAL(string("a"), output3[0]);
    CPPUNIT_ASSERT_EQUAL(string(""), output3[1]);

    auto output4 = string_split(",a", ',');
    CPPUNIT_ASSERT_EQUAL(2, (int)output4.size());
    CPPUNIT_ASSERT_EQUAL(string(""), output4[0]);
    CPPUNIT_ASSERT_EQUAL(string("a"), output4[1]);

    auto output5 = string_split("", ',');
    CPPUNIT_ASSERT_EQUAL(1, (int)output5.size());
    CPPUNIT_ASSERT_EQUAL(string(""), output5[0]);
  }

  void test_string_strip() {
    string ret = string_strip("");
    CPPUNIT_ASSERT_EQUAL(string(""), ret);

    ret = string_strip(" abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("  abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("\n\r\t abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("abc ");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("abc  ");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("abc \n\t\r");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("   abc ");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);

    ret = string_strip("abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), ret);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringUtilsTest);
}

