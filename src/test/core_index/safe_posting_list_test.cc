#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "core/index/posting_list.h"
#include "core/index/btree_posting_list.h"
#include "core/index/safe_posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"
#include "../core_reader/mock_reader.h"

namespace redgiant {
class SafePostingListTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SafePostingListTest);
  CPPUNIT_TEST(test_empty);
  CPPUNIT_TEST(test_read);
  CPPUNIT_TEST(test_update_apply);
  CPPUNIT_TEST(test_remove_apply);
  CPPUNIT_TEST(test_apply_read);
  CPPUNIT_TEST_SUITE_END();

public:
  SafePostingListTest() = default;
  virtual ~SafePostingListTest() = default;

protected:
  void test_empty() {
    bool empty;
    auto plist_1 = create_case_1();
    empty = plist_1->empty();
    CPPUNIT_ASSERT_EQUAL(false, empty);

    auto plist_empty = create_case_empty();
    empty = plist_empty->empty();
    CPPUNIT_ASSERT_EQUAL(true, empty);

    // with pending changes, not empty
    plist_empty->update(1, 1);
    empty = plist_empty->empty();
    CPPUNIT_ASSERT_EQUAL(false, empty);

    // write pending changes, not empty
    plist_empty->apply();
    empty = plist_empty->empty();
    CPPUNIT_ASSERT_EQUAL(false, empty);
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

  void test_update_apply() {
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

    // not applied yet
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

    // applied
    plist_1->apply();
    reader = create_reader_shared(plist_1);
    results = read_all(*reader);
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

  void test_remove_apply() {
    auto plist_1 = create_case_1();
    int ret;
    // remove success
    ret = plist_1->remove(8);
    CPPUNIT_ASSERT_EQUAL(1, ret);

    // remove fail: not found
    ret = plist_1->remove(9);
    CPPUNIT_ASSERT_EQUAL(0, ret);

    // not applied yet
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

    // applied
    plist_1->apply();
    reader = create_reader_shared(plist_1);
    results = read_all(*reader);
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

  void test_apply_read() {
    auto plist_1 = create_case_1();
    // update success
    plist_1->update(4, 1);
    plist_1->update(5, 1);
    plist_1->update(6, 2);
    plist_1->update(8, 2);
    // remove success
    plist_1->remove(4);
    plist_1->remove(5);
    plist_1->remove(10);

    // not applied yet
    auto reader = create_reader_shared(plist_1);

    // apply
    plist_1->apply();

    // read: will read the results before apply
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

    // read again
    reader = create_reader_shared(plist_1);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(8, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(2, results[1].second);
    CPPUNIT_ASSERT_EQUAL(6, results[2].first);
    CPPUNIT_ASSERT_EQUAL(2, results[2].second);
    CPPUNIT_ASSERT_EQUAL(8, results[3].first);
    CPPUNIT_ASSERT_EQUAL(2, results[3].second);
  }

private:
  std::shared_ptr<PostingList<int, int>> create_case_empty() {
    return std::make_shared<SafePostingList<int, int>>(create_factory());
  }

  std::shared_ptr<PostingList<int, int>> create_case_1() {
    std::unique_ptr<PostingListReader<int, int>> raw_reader (
        new MockReader<int, int>({ // upper bound: 8
          {1, 8}, {2, 2}, {5, 4}, {8, 4}, {10, 2}
        }));
    // create with a factory and an existing instance
    return std::make_shared<SafePostingList<int, int>>(create_factory(),
        create_factory()->create_posting_list(std::move(raw_reader)));
  }

  virtual std::unique_ptr<PostingListFactory<int, int>> create_factory() {
    return std::unique_ptr<PostingListFactory<int, int>>(new BTreePostingListFactory<int, int>());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SafePostingListTest);

} /* namespace redgiant */
