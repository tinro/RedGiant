#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils/cached_buffer.h"

namespace redgiant {

class MockBufferedItem {
public:
  static size_t alloc_count;
  static size_t release_count;

  MockBufferedItem() {
    ++alloc_count;
  }

  ~MockBufferedItem() {
    ++release_count;
  }
};

size_t MockBufferedItem::alloc_count = 0;
size_t MockBufferedItem::release_count = 0;

class CachedBufferTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CachedBufferTest);
  CPPUNIT_TEST(test_size);
  CPPUNIT_TEST(test_alloc);
  CPPUNIT_TEST_SUITE_END();

public:
  CachedBufferTest() = default;
  virtual ~CachedBufferTest() = default;

protected:
  void test_size() {
    CachedBuffer<int> buffer(256);
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.alloc(100);
    CPPUNIT_ASSERT_EQUAL(100, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() >= 100);
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.alloc(200);
    CPPUNIT_ASSERT_EQUAL(200, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() >= 200);
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.clear();
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.alloc(100);
    CPPUNIT_ASSERT_EQUAL(100, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() >= 100);
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.alloc(300);
    CPPUNIT_ASSERT_EQUAL(300, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() >= 300);

    buffer.clear();
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.alloc(100);
    CPPUNIT_ASSERT_EQUAL(100, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() >= 100);
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);

    buffer.clear();
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer.size());
    CPPUNIT_ASSERT(buffer.cached_size() <= 256);
  }

  void test_alloc() {
    MockBufferedItem::alloc_count = 0;
    MockBufferedItem::release_count = 0;

    CachedBuffer<MockBufferedItem> *buffer = new CachedBuffer<MockBufferedItem>(256);
    // not allocated by default
    CPPUNIT_ASSERT_EQUAL(0, (int)MockBufferedItem::alloc_count);
    CPPUNIT_ASSERT_EQUAL(0, (int)MockBufferedItem::release_count);

    buffer->alloc(100);
    // alloc (at least) 100 items
    CPPUNIT_ASSERT_EQUAL(100, (int)buffer->size());
    CPPUNIT_ASSERT(MockBufferedItem::alloc_count - MockBufferedItem::release_count >= 100);

    buffer->alloc(200);
    // alloc (at least) 200 items
    CPPUNIT_ASSERT_EQUAL(200, (int)buffer->size());
    CPPUNIT_ASSERT(MockBufferedItem::alloc_count - MockBufferedItem::release_count >= 200);

    buffer->clear();
    // released all items (except cached)
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer->size());
    CPPUNIT_ASSERT(MockBufferedItem::alloc_count - MockBufferedItem::release_count <= 256);

    buffer->alloc(300);
    CPPUNIT_ASSERT_EQUAL(300, (int)buffer->size());
    CPPUNIT_ASSERT(MockBufferedItem::alloc_count - MockBufferedItem::release_count >= 300);

    buffer->clear();
    CPPUNIT_ASSERT_EQUAL(0, (int)buffer->size());
    CPPUNIT_ASSERT(MockBufferedItem::alloc_count - MockBufferedItem::release_count <= 256);

    delete buffer;
    // assert all memory are released
    CPPUNIT_ASSERT_EQUAL(0, (int)(MockBufferedItem::alloc_count - MockBufferedItem::release_count));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CachedBufferTest);

} /* namespace redgiant */

