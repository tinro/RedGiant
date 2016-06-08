#include "core/impl/point_index_impl.h"
#include "core/impl/point_index_impl-inl.h"

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
template class PointIndexImpl<MockTraits>;
typedef PointIndexImpl<MockTraits> MockPointIndex;

class PointIndexImplTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PointIndexImplTest);
  CPPUNIT_TEST(test_update);
  CPPUNIT_TEST(test_batch_update);
  CPPUNIT_TEST(test_dump_restore);
  CPPUNIT_TEST_SUITE_END();

public:
  PointIndexImplTest() = default;
  virtual ~PointIndexImplTest() = default;

protected:
  void test_update() {
    auto index = create_case_empty();
    int ret;
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10/10/2
    ret = index->update(1, 101, 1, 10);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->update(1, 102, 2, 10);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->update(1, 103, 3, 2);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    ret = index->update(3, 101, 3, 20);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->update(3, 103, 5, 20);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->update(3, 105, 7, 20);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // doc (99): (103, 1), (110, 1), expire at 15/99
    ret = index->update(99, 103, 1, 15);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->update(99, 110, 1, 99);
    CPPUNIT_ASSERT_EQUAL(1, ret);
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

    // test expire
    index->apply(2);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[1].first);
    CPPUNIT_ASSERT_EQUAL(1, results[1].second);

    // more expires
    index->apply(15);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);

    // update doc 3, (102, 1), expire at 30
    ret = index->update(3, 102, 1, 30);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // update doc 3, (105, 9), expire at 30
    ret = index->update(3, 105, 9, 30);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // update doc 99: (105, 1), expire at 30
    ret = index->update(99, 105, 1, 30);
    CPPUNIT_ASSERT_EQUAL(1, ret);

    // apply changes with expiration, doc 1 expires totally
    // doc 3 and 99 has some terms left
    index->apply(25);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(3, (int)index->get_term_count());

    // term 102, two results
    reader = index->peek(102);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 1
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(1, results[0].second);

    // term 103, no result (all expired)
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
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10/10/2
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    // doc (99): (103, 1), (110, 1), expire at 15/99
    ret = index->batch_update(std::vector<MockPointIndex::PointTuple> {
      MockPointIndex::PointTuple {1, 101, 1, 10},
      MockPointIndex::PointTuple {1, 102, 2, 10},
      MockPointIndex::PointTuple {1, 103, 3, 2},
      MockPointIndex::PointTuple {3, 101, 3, 20},
      MockPointIndex::PointTuple {3, 103, 5, 20},
      MockPointIndex::PointTuple {3, 105, 7, 20},
      MockPointIndex::PointTuple {99, 103, 1, 15},
      MockPointIndex::PointTuple {99, 110, 1, 99}
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

    // update doc 3, (101, 1), expire at 30
    ret = index->batch_update(std::vector<MockPointIndex::PointTuple> {
      MockPointIndex::PointTuple {3, 102, 1, 30},
      MockPointIndex::PointTuple {3, 105, 9, 30},
      MockPointIndex::PointTuple {99, 105, 1, 30},
      MockPointIndex::PointTuple {99, 110, 1, 99}
    });
    CPPUNIT_ASSERT_EQUAL(4, ret);

    // apply changes with expiration, doc 1 expires totally
    // doc 3 and 99 has some terms left
    index->apply(25);
    // changes applied
    CPPUNIT_ASSERT_EQUAL(3, (int)index->get_term_count());

    // term 102, two results
    reader = index->peek(102);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 1
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(1, results[0].second);

    // term 103, no result (all expired)
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

  void test_dump_restore() {
    auto index = create_case_1();
    std::string snapshot_file_name = "test.snapshot.dump";
    index->dump(SnapshotDumper(snapshot_file_name));

    // re-create index
    index = std::make_shared<MockPointIndex>(
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

    // apply changes with expiration, only 4 points left (with 4 terms).
    index->apply(15);
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    reader = index->peek(103);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(5, results[0].second);
  }

private:
  std::shared_ptr<MockPointIndex> create_case_empty() {
    std::shared_ptr<MockPointIndex> index = std::make_shared<MockPointIndex>(100, 1000);
    return index;
  }

  std::shared_ptr<MockPointIndex> create_case_1() {
    std::shared_ptr<MockPointIndex> index = std::make_shared<MockPointIndex>(100, 1000);
    // doc (1): (101, 1), (102, 2), (103, 3). expire at 10/10/2
    index->update(1, 101, 1, 10);
    index->update(1, 102, 2, 10);
    index->update(1, 103, 3, 2);
    // doc (3): (101, 3), (103, 5), (105, 7). expire at 20
    index->update(3, 101, 3, 20);
    index->update(3, 103, 5, 20);
    index->update(3, 105, 7, 20);
    // doc (99): (103, 1), (110, 1), expire at 15/99
    index->update(99, 103, 1, 15);
    index->update(99, 110, 1, 99);
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

CPPUNIT_TEST_SUITE_REGISTRATION(PointIndexImplTest);

} /* namespace redgiant */
