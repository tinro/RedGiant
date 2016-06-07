#include "core/impl/freezable_posting_list.h"

#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../core_reader/mock_reader.h"

#include "core/index/posting_list.h"
#include "core/index/btree_posting_list.h"
#include "core/reader/posting_list_reader.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
class FreezablePostingListTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FreezablePostingListTest);
  CPPUNIT_TEST(test_read);
  //CPPUNIT_TEST(test_freeze);
  //CPPUNIT_TEST(test_get_instance);
  CPPUNIT_TEST_SUITE_END();

public:
  FreezablePostingListTest() = default;
  virtual ~FreezablePostingListTest() = default;

protected:
  void test_read() {
    auto fplist = create_case_1();

    // not frozen, cannot read
    auto reader = fplist->create_reader(fplist);
    CPPUNIT_ASSERT(nullptr == reader);

    fplist->freeze();
    reader = fplist->create_reader(fplist);
    CPPUNIT_ASSERT(!!reader);

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

  void test_freeze() {
    auto fplist = create_case_empty();
    CPPUNIT_ASSERT_EQUAL(true, fplist->empty());

    int ret;
    ret = fplist->update(1, 1);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = fplist->update(2, 2);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = fplist->update(3, 4);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = fplist->remove(2);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // update fail: invalid key
    ret = fplist->update(0, 1);
    CPPUNIT_ASSERT_EQUAL(0, ret);

    // (1,1), (3,4)
    fplist->freeze();

    // do not allow
    ret = fplist->update(5, 6);
    CPPUNIT_ASSERT_EQUAL(0, ret);
    // do not allow
    ret = fplist->remove(1);
    CPPUNIT_ASSERT_EQUAL(0, ret);

    auto reader = fplist->create_reader(fplist);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(1, results[0].second);
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(4, results[1].second);
  }

  void test_get_instance() {
    auto fplist = create_case_1();

    // not frozen, empty
    auto plist = fplist->get_instance();
    CPPUNIT_ASSERT(nullptr == plist);

    fplist->freeze();
    plist = fplist->get_instance();
    CPPUNIT_ASSERT(!!plist);

    auto reader = plist->create_reader(plist);
    // should get the same results
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

private:
  std::shared_ptr<FreezablePostingList<int, int>> create_case_empty() {
    return std::make_shared<FreezablePostingList<int, int>>(*create_factory());
  }

  std::shared_ptr<FreezablePostingList<int, int>> create_case_1() {
    std::unique_ptr<PostingListReader<int, int>> raw_reader (
        new MockReader<int, int>({ // upper bound: 8
          {1, 8}, {2, 2}, {5, 4}, {8, 4}, {10, 2}
        }));
    // create with a factory and an existing instance
    return std::make_shared<FreezablePostingList<int, int>>(*create_factory(), std::move(raw_reader));
  }

  virtual std::unique_ptr<PostingListFactory<int, int>> create_factory() {
    return std::unique_ptr<PostingListFactory<int, int>>(new BTreePostingListFactory<int, int>());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FreezablePostingListTest);

} /* namespace redgiant */
