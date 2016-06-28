#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_weight.h"
#include "../core_reader/mock_reader.h"

#include "core/index/posting_list.h"
#include "core/index/map_posting_list.h"
#include "core/index/btree_posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
class PostingListTest: public CppUnit::TestFixture {
public:
  PostingListTest() = default;
  virtual ~PostingListTest() = default;

protected:
  void test_empty() {
    bool empty;
    auto plist_1 = create_case_1();
    empty = plist_1->empty();
    CPPUNIT_ASSERT_EQUAL(false, empty);

    auto plist_empty = create_case_empty();
    empty = plist_empty->empty();
    CPPUNIT_ASSERT_EQUAL(true, empty);
  }

  void test_read() {
    auto plist_1 = create_case_1();
    auto reader = create_reader_shared(plist_1);
    std::vector<std::pair<int, int>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(5, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(8, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(2, results[1].second);
    CPPUNIT_ASSERT_EQUAL(5, results[2].first);
    CPPUNIT_ASSERT_EQUAL(4, results[2].second);
    CPPUNIT_ASSERT_EQUAL(8, results[3].first);
    CPPUNIT_ASSERT_EQUAL(4, results[3].second);
    CPPUNIT_ASSERT_EQUAL(10, results[4].first);
    CPPUNIT_ASSERT_EQUAL(2, results[4].second);
  }

  void test_read_2() {
    auto plist_2 = create_case_2();
    auto reader = create_reader_shared(plist_2);
    std::vector<std::pair<int, MockWeight>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(5, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(2, results[0].second.w1);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second.w2);
  }

  void test_update() {
    auto plist_1 = create_case_1();
    int ret;
    // update success
    ret = plist_1->update(5, 1);
    CPPUNIT_ASSERT_EQUAL(1, ret);

    // update success: insert
    ret = plist_1->update(6, 2);
    CPPUNIT_ASSERT_EQUAL(1, ret);

    // update fail: invalid key
    ret = plist_1->update(0, 1);
    CPPUNIT_ASSERT_EQUAL(0, ret);

    auto reader = create_reader_shared(plist_1);
    std::vector<std::pair<int, int>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(6, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(8, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(2, results[1].second);
    CPPUNIT_ASSERT_EQUAL(5, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);
    CPPUNIT_ASSERT_EQUAL(6, results[3].first);
    CPPUNIT_ASSERT_EQUAL(2, results[3].second);
    CPPUNIT_ASSERT_EQUAL(8, results[4].first);
    CPPUNIT_ASSERT_EQUAL(4, results[4].second);
    CPPUNIT_ASSERT_EQUAL(10, results[5].first);
    CPPUNIT_ASSERT_EQUAL(2, results[5].second);
  }

  void test_remove() {
    auto plist_1 = create_case_1();
    int ret;
    // remove success
    ret = plist_1->remove(8);
    CPPUNIT_ASSERT_EQUAL(1, ret);

    // remove fail: not found
    ret = plist_1->remove(9);
    CPPUNIT_ASSERT_EQUAL(0, ret);

    auto reader = create_reader_shared(plist_1);
    std::vector<std::pair<int, int>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(8, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(2, results[1].second);
    CPPUNIT_ASSERT_EQUAL(5, results[2].first);
    CPPUNIT_ASSERT_EQUAL(4, results[2].second);
    CPPUNIT_ASSERT_EQUAL(10, results[3].first);
    CPPUNIT_ASSERT_EQUAL(2, results[3].second);
  }

  void test_size() {
    auto plist_1 = create_case_1();
    auto reader = create_reader_shared(plist_1);
    CPPUNIT_ASSERT_EQUAL(5, (int)reader->size());
  }

  void test_upper_bound() {
    auto plist_1 = create_case_1();
    auto reader = create_reader_shared(plist_1);
    CPPUNIT_ASSERT_EQUAL(8, (int)reader->upper_bound());
  }

  // upper bound calculation of user defined types may be tricky
  void test_upper_bound_2() {
    auto plist_2 = create_case_2();
    auto reader = create_reader_shared(plist_2);
    auto upper_bound = reader->upper_bound();
    CPPUNIT_ASSERT_EQUAL(9, upper_bound.w1);
    CPPUNIT_ASSERT_EQUAL(8, upper_bound.w2);
  }

  void test_threshold() {
    auto plist_1 = create_case_1();
    auto reader = create_reader_shared(plist_1);
    reader->threshold(10); // no effect
  }

  virtual std::unique_ptr<PostingListFactory<int, int>> create_factory() = 0;

  virtual std::unique_ptr<PostingListFactory<int, MockWeight>> create_factory_weight() = 0;

private:
  std::shared_ptr<PostingList<int, int>> create_case_empty() {
    return create_factory()->create_posting_list();
  }

  std::shared_ptr<PostingList<int, int>> create_case_1() {
    auto raw_reader = std::make_unique<MockReader<int, int>>(
        MockReader<int, int>::PostingVec { // upper bound: 8
          {1, 8}, {2, 2}, {5, 4}, {8, 4}, {10, 2}
        });
    return create_factory()->create_posting_list(std::move(raw_reader));
  }

  std::shared_ptr<PostingList<int, MockWeight>> create_case_2() {
    auto raw_reader = std::make_unique<MockReader<int, MockWeight>>(
        MockReader<int, MockWeight>::PostingVec { // upper bound: {9, 8}
          {1, {2, 5}}, {2, {3, 7}}, {5, {1, 8}}, {8, {9, 2}}, {10, {3, 1}}
        });
    return create_factory_weight()->create_posting_list(std::move(raw_reader));
  }
};

class BTreePostingListTest: public PostingListTest {
  CPPUNIT_TEST_SUITE(BTreePostingListTest);
  CPPUNIT_TEST(test_empty);
  CPPUNIT_TEST(test_read);
  CPPUNIT_TEST(test_read_2);
  CPPUNIT_TEST(test_update);
  CPPUNIT_TEST(test_remove);
  CPPUNIT_TEST(test_size);
  CPPUNIT_TEST(test_upper_bound);
  CPPUNIT_TEST(test_upper_bound_2);
  CPPUNIT_TEST(test_threshold);
  CPPUNIT_TEST_SUITE_END();

public:
  BTreePostingListTest() = default;
  virtual ~BTreePostingListTest() = default;

protected:
  virtual std::unique_ptr<PostingListFactory<int, int>> create_factory() {
    return std::make_unique<BTreePostingListFactory<int, int>>();
  }

  virtual std::unique_ptr<PostingListFactory<int, MockWeight>> create_factory_weight() {
    return std::make_unique<BTreePostingListFactory<int, MockWeight>>();
  }
};

class MapPostingListTest: public PostingListTest {
  CPPUNIT_TEST_SUITE(MapPostingListTest);
  CPPUNIT_TEST(test_empty);
  CPPUNIT_TEST(test_read);
  CPPUNIT_TEST(test_read_2);
  CPPUNIT_TEST(test_update);
  CPPUNIT_TEST(test_remove);
  CPPUNIT_TEST(test_size);
  CPPUNIT_TEST(test_upper_bound);
  CPPUNIT_TEST(test_upper_bound_2);
  CPPUNIT_TEST(test_threshold);
  CPPUNIT_TEST_SUITE_END();

public:
  MapPostingListTest() = default;
  virtual ~MapPostingListTest() = default;

protected:
  virtual std::unique_ptr<PostingListFactory<int, int>> create_factory() {
    return std::make_unique<MapPostingListFactory<int, int>>();
  }

  virtual std::unique_ptr<PostingListFactory<int, MockWeight>> create_factory_weight() {
    return std::make_unique<MapPostingListFactory<int, MockWeight>>();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BTreePostingListTest);
CPPUNIT_TEST_SUITE_REGISTRATION(MapPostingListTest);

} /* namespace redgiant */
