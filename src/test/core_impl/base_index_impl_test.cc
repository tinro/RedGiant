#include "core/impl/base_index_impl.h"
#include "core/impl/base_index_impl-inl.h"

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
template class BaseIndexImpl<MockTraits>;
typedef BaseIndexImpl<MockTraits> MockBaseIndex;

class BaseIndexImplTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BaseIndexImplTest);
  CPPUNIT_TEST(test_update_internal);
  CPPUNIT_TEST(test_remove_internal);
  CPPUNIT_TEST(test_query);
  CPPUNIT_TEST(test_batch_query);
  CPPUNIT_TEST_SUITE_END();

public:
  BaseIndexImplTest() = default;
  virtual ~BaseIndexImplTest() = default;

protected:
  void test_update_internal() {
    auto index = create_case_empty();
    CPPUNIT_ASSERT_EQUAL(0, (int)index->index_.size());

    int ret;
    // make some update
    ret = index->create_update_internal(1, 101, 1);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->create_update_internal(3, 103, 5);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    ret = index->create_update_internal(1, 103, 3);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(0, (int)index->get_term_count());

    // apply changes
    index->apply_internal();
    // changes applied
    CPPUNIT_ASSERT_EQUAL(2, (int)index->get_term_count());

    auto reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 3, weight 5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(5, results[1].second);

    reader = index->peek(101);
    results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 1, weight 1
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(1, results[0].second);
  }

  void test_remove_internal() {
    auto index = create_case_1();
    CPPUNIT_ASSERT_EQUAL(5, (int)index->index_.size());

    int ret;
    // remove exist
    ret = index->remove_internal(1, 101);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    // remove non-exist
    ret = index->remove_internal(99, 101);
    CPPUNIT_ASSERT_EQUAL(0, ret);
    // batch remove
    ret = index->remove_internal(3, {101, 103, 110});
    CPPUNIT_ASSERT_EQUAL(2, ret);
    // changes not applied
    CPPUNIT_ASSERT_EQUAL(5, (int)index->get_term_count());

    // apply changes. term 101 get removed
    index->apply_internal();
    // changes applied
    CPPUNIT_ASSERT_EQUAL(4, (int)index->get_term_count());

    auto reader = index->peek(101);
    CPPUNIT_ASSERT(nullptr == reader);

    reader = index->peek(103);
    std::vector<std::pair<int, int>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 1, weight 3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3, results[0].second);
    // doc 99, weight 1
    CPPUNIT_ASSERT_EQUAL(99, results[1].first);
    CPPUNIT_ASSERT_EQUAL(1, results[1].second);
  }

  void test_query() {
    auto index = create_case_1();
    auto reader = index->query(103, *create_query(5));
    std::vector<std::pair<int, long>> results = read_all(*reader);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    // doc 1, score 3*5
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(15l, results[0].second);
    // doc 1, score 5*5
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(25l, results[1].second);
    // doc 99, weight 1*5
    CPPUNIT_ASSERT_EQUAL(99, results[2].first);
    CPPUNIT_ASSERT_EQUAL(5l, results[2].second);

    reader = index->query(199, *create_query(5));
    CPPUNIT_ASSERT(nullptr == reader);
  }

  void test_batch_query() {
    auto index = create_case_1();
    std::vector<typename MockBaseIndex::QueryPair<long>> queries;
    queries.emplace_back(105, create_query(5));
    queries.emplace_back(199, create_query(99));
    queries.emplace_back(101, create_query(3));
    auto readers = index->batch_query<long>(queries);

    // two terms
    CPPUNIT_ASSERT_EQUAL(2, (int)readers.size());

    // item 1, term 105
    CPPUNIT_ASSERT_EQUAL(105, readers[0].first);
    std::vector<std::pair<int, long>> results = read_all(*readers[0].second);
    // 1 doc in the reader
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    // doc 3, score 7*5
    CPPUNIT_ASSERT_EQUAL(3, results[0].first);
    CPPUNIT_ASSERT_EQUAL(35l, results[0].second);

    // item 2, term 101
    CPPUNIT_ASSERT_EQUAL(101, readers[1].first);
    results = read_all(*readers[1].second);
    // 2 docs in the reader
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    // doc 1, score 1*3
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(3l, results[0].second);
    // doc 1, score 3*3
    CPPUNIT_ASSERT_EQUAL(3, results[1].first);
    CPPUNIT_ASSERT_EQUAL(9l, results[1].second);
  }

private:
  std::shared_ptr<MockBaseIndex> create_case_empty() {
    std::shared_ptr<MockBaseIndex> index = std::make_shared<MockBaseIndex>(100);
    return index;
  }

  std::shared_ptr<MockBaseIndex> create_case_1() {
    std::shared_ptr<MockBaseIndex> index = std::make_shared<MockBaseIndex>(100);
    // doc (1): (101, 1), (102, 2), (103, 3)
    index->create_update_internal(1, 101, 1);
    index->create_update_internal(1, 102, 2);
    index->create_update_internal(1, 103, 3);
    // doc (3): (101, 3), (103, 5), (105, 7)
    index->create_update_internal(3, 101, 3);
    index->create_update_internal(3, 103, 5);
    index->create_update_internal(3, 105, 7);
    // doc (99): (103, 1), (110, 1)
    index->create_update_internal(99, 103, 1);
    index->create_update_internal(99, 110, 1);
    index->apply_internal();
    return index;
  }

  // output score is long
  std::unique_ptr<PostingListQuery<int, long, const int&>> create_query(long x) {
    return std::make_unique<DotProductQuery<int, long, const int&>>(x);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseIndexImplTest);

} /* namespace redgiant */
