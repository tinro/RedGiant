#include "core/impl/row_index_impl.h"
#include "core/impl/row_index_impl-inl.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_traits.h"
#include "../core_reader/mock_reader.h"
#include "core/query/dot_product_query.h"
#include "core/reader/reader_utils.h"

namespace redgiant {
// instantiate the template class
template class RowIndexImpl<MockTraits>;
typedef RowIndexImpl<MockTraits> MockRowIndex;

class RowIndexImplTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RowIndexImplTest);
  CPPUNIT_TEST(test_update);
  CPPUNIT_TEST(test_batch_update);
  CPPUNIT_TEST(test_remove);
  CPPUNIT_TEST(test_batch_remove);
  CPPUNIT_TEST(test_dump_restore);
  CPPUNIT_TEST_SUITE_END();

public:
  RowIndexImplTest() = default;
  virtual ~RowIndexImplTest() = default;

protected:
  void test_update() {
    auto index = create_case_empty();
    int ret;
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10
    ret = index->update(1, {{101, 1}, {102, 2}, {103, 3}}, 10);
    CPPUNIT_ASSERT_EQUAL(3, ret);
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    ret = index->update(3, {{101, 3}, {103, 5}, {105, 7}}, 20);
    CPPUNIT_ASSERT_EQUAL(3, ret);
    // doc (99): (103, 1), (110, 1), expire at 15
    ret = index->update(99, {{103, 1}, {110, 1}}, 15);
    CPPUNIT_ASSERT_EQUAL(2, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(0, (int)index->get_term_count());

    // apply changes
    index->apply(1);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);

    // apply changes with expiration, doc 1 and 99 expired
    index->apply(15);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(3, (int)index->get_term_count());

    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);

    // update doc (3): (101, 8), (105, 9), (110, 10). expire at 30
    ret = index->update(3, {{101, 8}, {105, 9}, {110, 10}}, 30);
    CPPUNIT_ASSERT_EQUAL(3, ret);
    // update doc (99): (105, 1), (108, 1), expire at 30
    ret = index->update(99, {{105, 1}, {108, 1}}, 30);
    CPPUNIT_ASSERT_EQUAL(2, ret);

    // apply changes with expiration, doc 3 and 99 should not expire
    index->apply(25);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    // term 103, no result
    reader = index->peek(103);
    CPPUNIT_ASSERT(nullptr == reader);

    // term 105, two results
    reader = index->peek(105);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 3, weight 9
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(9, results[0].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[1].first);
    CPPUNIT_ASSERT_EQUAL(1, results[1].second);
  }

  void test_batch_update() {
    auto index = create_case_empty();
    int ret;
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    // doc (99): (103, 1), (110, 1), expire at 15
    ret = index->batch_update(std::vector<MockRowIndex::RowTuple> {
      MockRowIndex::RowTuple {1, {{101, 1}, {102, 2}, {103, 3}}, 10},
      MockRowIndex::RowTuple {3, {{101, 3}, {103, 5}, {105, 7}}, 20},
      MockRowIndex::RowTuple {99, {{103, 1}, {110, 1}}, 15}
    });
    CPPUNIT_ASSERT_EQUAL(8, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(0, (int)index->get_term_count());

    // apply changes
    index->apply(1);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);

    // update doc (3): (101, 8), (105, 9), (110, 10). expire at 30
    // update doc (99): (105, 1), (108, 1), expire at 30
    ret = index->batch_update(std::vector<MockRowIndex::RowTuple> {
      MockRowIndex::RowTuple {3, {{101, 8}, {105, 9}, {110, 10}}, 30},
      MockRowIndex::RowTuple {99, {{105, 1}, {108, 1}}, 30}
    });
    CPPUNIT_ASSERT_EQUAL(5, ret);

    // apply changes with expiration, doc 1 expired, doc 3 and 99 should not expire
    index->apply(25);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    // term 103, no result
    reader = index->peek(103);
    CPPUNIT_ASSERT(nullptr == reader);

    // term 105, two results
    reader = index->peek(105);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 3, weight 9
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(9, results[0].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[1].first);
    CPPUNIT_ASSERT_EQUAL(1, results[1].second);
  }

  void test_remove() {
    auto index = create_case_1();
    CPPUNIT_ASSERT_EQUAL(5, (int)index->index_.size());

    int ret;
    // remove doc 99
    ret = index->remove(99);
    CPPUNIT_ASSERT_EQUAL(2, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    // docs 1, 3 and 99
    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);

    // apply changes
    index->apply(1);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    // docs 1 and 3
    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
  }

  void test_batch_remove() {
    auto index = create_case_1();
    CPPUNIT_ASSERT_EQUAL(5, (int)index->index_.size());

    int ret;
    // remove doc 99
    ret = index->batch_remove({1, 99});
    CPPUNIT_ASSERT_EQUAL(5, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    // docs 1, 3 and 99
    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);

    // apply changes
    index->apply(1);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(3, (int)index->get_term_count());

    // docs 1 and 3
    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);
  }

  void test_dump_restore() {
    auto index = create_case_1();
    std::string snapshot_file_name = "test.snapshot.dump";
    index->dump(SnapshotDumper(snapshot_file_name));

    // re-create index
    index = std::make_shared<MockRowIndex>(
        100, 1000, SnapshotLoader(snapshot_file_name));

    // index load successfully
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());
    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(1, results[2].second);

    // apply changes with expiration, doc 1 and 99 expired
    index->apply(15);
    CPPUNIT_ASSERT_EQUAL(3, (int)index->get_term_count());

    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);
  }

private:
  std::shared_ptr<MockRowIndex> create_case_empty() {
    std::shared_ptr<MockRowIndex> index = std::make_shared<MockRowIndex>(100, 1000);
    return index;
  }

  std::shared_ptr<MockRowIndex> create_case_1() {
    std::shared_ptr<MockRowIndex> index = std::make_shared<MockRowIndex>(100, 1000);
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10
    index->update(1, {{101, 1}, {102, 2}, {103, 3}}, 10);
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    index->update(3, {{101, 3}, {103, 5}, {105, 7}}, 20);
    // doc (99): (103, 1), (110, 1), expire at 15
    index->update(99, {{103, 1}, {110, 1}}, 15);
    // apply changes
    index->apply(1);
    return index;
  }

  // output score is long
  std::unique_ptr<PostingListQuery<int, long, const int&>> create_query(long x) {
    return std::unique_ptr<PostingListQuery<int, long, const int&>>(
        new DotProductQuery<int, long, const int&>(x));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RowIndexImplTest);

} /* namespace redgiant */
