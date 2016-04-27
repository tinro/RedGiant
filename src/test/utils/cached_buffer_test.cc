#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils/cached_buffer.h"
#include "utils/logger.h"

namespace redgiant {

class CachedBufferTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CachedBufferTest);
  CPPUNIT_TEST(test_rebuffer);
  CPPUNIT_TEST_SUITE_END();

public:
  CachedBufferTest() = default;
  virtual ~CachedBufferTest() = default;

protected:
  void test_rebuffer() {
    CachedBuffer<int> buffer(4096);
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());

    buffer.alloc(1024);
    CPPUNIT_ASSERT_EQUAL(1024, (int)buffer.size());

    buffer.alloc(2048);
    CPPUNIT_ASSERT_EQUAL(2048, (int)buffer.size());

    buffer.alloc(8192);
    CPPUNIT_ASSERT_EQUAL(8192, (int)buffer.size());

    buffer.clear();
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CachedBufferTest);

} /* namespace redgiant */

