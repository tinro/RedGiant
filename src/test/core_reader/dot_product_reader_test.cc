#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_reader.h"

#include "core/reader/reader_utils.h"
#include "core/reader/dot_product_reader.h"

namespace redgiant {
class DotProductReaderTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DotProductReaderTest);
  CPPUNIT_TEST(test_read_all);
  CPPUNIT_TEST(test_upper_bound);
  CPPUNIT_TEST(test_size);
  CPPUNIT_TEST_SUITE_END();

public:
  DotProductReaderTest() = default;
  virtual ~DotProductReaderTest() = default;

protected:
  void test_read_all() {
    auto reader = create_case_1(5);
    std::vector<std::pair<int, int>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(5, (int)results.size());
    // all values multiplied by 5
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(40, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(10, results[1].second);
    CPPUNIT_ASSERT_EQUAL(5, results[2].first);
    CPPUNIT_ASSERT_EQUAL(20, results[2].second);
    CPPUNIT_ASSERT_EQUAL(8, results[3].first);
    CPPUNIT_ASSERT_EQUAL(20, results[3].second);
    CPPUNIT_ASSERT_EQUAL(10, results[4].first);
    CPPUNIT_ASSERT_EQUAL(10, results[4].second);
  }

  void test_upper_bound() {
    auto reader = create_case_1(4);
    int upper_bound = reader->upper_bound();
    CPPUNIT_ASSERT_EQUAL(32, upper_bound);
  }

  void test_size() {
    auto reader = create_case_1(3);
    int size = reader->size();
    CPPUNIT_ASSERT_EQUAL(5, size);
  }

private:
  std::unique_ptr<DotProductReader<int, int, int>> create_case_1(int query) {
    auto raw_reader = std::make_unique<MockReader<int, int>>({ // upper bound: 8
          {1, 8}, {2, 2}, {5, 4}, {8, 4}, {10, 2}
        });

    return std::make_unique<DotProductReader<int, int, int>>(std::move(raw_reader), query);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DotProductReaderTest);

} /* namespace redgiant */
