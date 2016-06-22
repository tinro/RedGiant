#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "mock_reader.h"

#include "core/reader/reader_utils.h"
#include "core/reader/wand_reader.h"
#include "core/reader/wand_reader-inl.h"

namespace redgiant {
class WandReaderTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(WandReaderTest);
  CPPUNIT_TEST(test_read_all);
  CPPUNIT_TEST(test_next_read);
  CPPUNIT_TEST(test_upper_bound);
  CPPUNIT_TEST(test_find_pivot);
  CPPUNIT_TEST(test_remove_term);
  CPPUNIT_TEST(test_move_term);
  CPPUNIT_TEST(test_move_term_no_move);
  CPPUNIT_TEST(test_size);
  CPPUNIT_TEST_SUITE_END();

public:
  WandReaderTest() = default;
  virtual ~WandReaderTest() = default;

protected:
  void test_read_all() {
    auto reader = create_case_1();
    std::vector<std::pair<int, int>> results = read_all(*reader);

    CPPUNIT_ASSERT_EQUAL(8, (int)results.size());
    CPPUNIT_ASSERT_EQUAL(1, results[0].first);
    CPPUNIT_ASSERT_EQUAL(10, results[0].second);
    CPPUNIT_ASSERT_EQUAL(2, results[1].first);
    CPPUNIT_ASSERT_EQUAL(2, results[1].second);
    CPPUNIT_ASSERT_EQUAL(3, results[2].first);
    CPPUNIT_ASSERT_EQUAL(2, results[2].second);
    CPPUNIT_ASSERT_EQUAL(5, results[3].first);
    CPPUNIT_ASSERT_EQUAL(20, results[3].second);
    CPPUNIT_ASSERT_EQUAL(7, results[4].first);
    CPPUNIT_ASSERT_EQUAL(2, results[4].second);
    CPPUNIT_ASSERT_EQUAL(8, results[5].first);
    CPPUNIT_ASSERT_EQUAL(5, results[5].second);
    CPPUNIT_ASSERT_EQUAL(9, results[6].first);
    CPPUNIT_ASSERT_EQUAL(22, results[6].second);
    CPPUNIT_ASSERT_EQUAL(10, results[7].first);
    CPPUNIT_ASSERT_EQUAL(7, results[7].second);
  }

  void test_next_read() {
    auto reader = create_case_1();
    // sorted_indexes_: 0, 1, 2, 3
    CPPUNIT_ASSERT_EQUAL(4, (int)reader->sorted_indexes_.size());
    CPPUNIT_ASSERT_EQUAL(0, (int)reader->sorted_indexes_[0]);
    CPPUNIT_ASSERT_EQUAL(1, (int)reader->sorted_indexes_[1]);
    CPPUNIT_ASSERT_EQUAL(2, (int)reader->sorted_indexes_[2]);
    CPPUNIT_ASSERT_EQUAL(3, (int)reader->sorted_indexes_[3]);

    int id = 0;
    int score = 0;
    // next id: 1
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(1, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(10, score);
    // next id: 2
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(2, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(2, score);

    // set threshold: 5
    reader->threshold(5);

    // 3 is not skipped since the upper bound is 12
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(3, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(2, score);

    // next id: 5
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(5, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(20, score);

    // set threshold: 10
    reader->threshold(10);

    // 7 will be skipped since the upper bound is 2
    // 8 will be skipped since the upper bound is 10
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(9, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(22, score);

    // 10 will be evaluated since the upper bound is 18
    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(10, id);
    score = reader->read();
    CPPUNIT_ASSERT_EQUAL(7, score);

    id = reader->next(id);
    CPPUNIT_ASSERT_EQUAL(0, id);
  }

  void test_upper_bound() {
    auto reader = create_case_1();
    int upper_bound = reader->upper_bound();
    CPPUNIT_ASSERT_EQUAL(30, upper_bound);
  }

  void test_find_pivot() {
    auto reader = create_case_1();

    int pivot;
    // upper bounds: 8, 2, 10, 10
    reader->threshold(8);
    pivot = reader->find_pivot(0);
    CPPUNIT_ASSERT_EQUAL(1, pivot);

    // find pivot from the third place
    reader->threshold(10);
    pivot = reader->find_pivot(2);
    CPPUNIT_ASSERT_EQUAL(2, pivot);

    // not found, will skip all
    reader->threshold(100);
    pivot = reader->find_pivot(0);
    CPPUNIT_ASSERT_EQUAL(4, pivot);
  }

  void test_remove_term() {
    auto reader = create_case_1();

    // remove_term will not call next() on readers, so we can set
    // the internal status of WandReader
    // sorted_indexes_:  0, 1, 2, 3
    // internal upper_bounds_: 8, 2, 10, 10
    // internal acc_upper_bounds_: 8, 10, 20, 30
    reader->reader_cursors_ = { 2, 3, 5, 9 };
    reader->remove_term(1);

    // sorted_indexes_: 0, 2, 3
    CPPUNIT_ASSERT_EQUAL(3, (int)reader->sorted_indexes_.size());
    CPPUNIT_ASSERT_EQUAL(0, (int)reader->sorted_indexes_[0]);
    CPPUNIT_ASSERT_EQUAL(2, (int)reader->sorted_indexes_[1]);
    CPPUNIT_ASSERT_EQUAL(3, (int)reader->sorted_indexes_[2]);

    // acc_upper_bounds_: 8, 18, 28
    CPPUNIT_ASSERT_EQUAL(3,  (int)reader->acc_upper_bounds_.size());
    CPPUNIT_ASSERT_EQUAL(8,  reader->acc_upper_bounds_[0]);
    CPPUNIT_ASSERT_EQUAL(18, reader->acc_upper_bounds_[1]);
    CPPUNIT_ASSERT_EQUAL(28, reader->acc_upper_bounds_[2]);
  }

  void test_move_term() {
    auto reader = create_case_1();

    // remove_term will not call next() on readers, so we can set
    // the internal status of WandReader
    // sorted_indexes_:  0, 1, 2, 3
    // internal upper_bounds_: 8, 2, 10, 10
    // internal acc_upper_bounds_: 8, 10, 20, 30
    reader->reader_cursors_ = { 5, 5, 5, 9 };

    // assuming we just iterated the second term
    reader->reader_cursors_[1] = 7;
    reader->move_term(1);

    // sorted_indexes_: 0, 2, 1, 3
    CPPUNIT_ASSERT_EQUAL(4, (int)reader->sorted_indexes_.size());
    CPPUNIT_ASSERT_EQUAL(0, (int)reader->sorted_indexes_[0]);
    CPPUNIT_ASSERT_EQUAL(2, (int)reader->sorted_indexes_[1]);
    CPPUNIT_ASSERT_EQUAL(1, (int)reader->sorted_indexes_[2]);
    CPPUNIT_ASSERT_EQUAL(3, (int)reader->sorted_indexes_[3]);

    // acc_upper_bounds_: 8, 18, 20, 30
    CPPUNIT_ASSERT_EQUAL(8,  reader->acc_upper_bounds_[0]);
    CPPUNIT_ASSERT_EQUAL(18, reader->acc_upper_bounds_[1]);
    CPPUNIT_ASSERT_EQUAL(20, reader->acc_upper_bounds_[2]);
    CPPUNIT_ASSERT_EQUAL(30, reader->acc_upper_bounds_[3]);
  }

  void test_move_term_no_move() {
    auto reader = create_case_1();

    // remove_term will not call next() on readers, so we can set
    // the internal status of WandReader
    // sorted_indexes_:  0, 1, 2, 3
    // internal upper_bounds_: 8, 2, 10, 10
    // internal acc_upper_bounds_: 8, 10, 20, 30
    reader->reader_cursors_ = { 2, 3, 5, 9 };

    // assuming we just iterated the second term
    reader->reader_cursors_[1] = 5;
    reader->move_term(1);

    // sorted_indexes_: 0, 1, 2, 3
    CPPUNIT_ASSERT_EQUAL(4, (int)reader->sorted_indexes_.size());
    CPPUNIT_ASSERT_EQUAL(0, (int)reader->sorted_indexes_[0]);
    CPPUNIT_ASSERT_EQUAL(1, (int)reader->sorted_indexes_[1]);
    CPPUNIT_ASSERT_EQUAL(2, (int)reader->sorted_indexes_[2]);
    CPPUNIT_ASSERT_EQUAL(3, (int)reader->sorted_indexes_[3]);

    // acc_upper_bounds_: 8, 10, 20, 30
    CPPUNIT_ASSERT_EQUAL(8,  reader->acc_upper_bounds_[0]);
    CPPUNIT_ASSERT_EQUAL(10, reader->acc_upper_bounds_[1]);
    CPPUNIT_ASSERT_EQUAL(20, reader->acc_upper_bounds_[2]);
    CPPUNIT_ASSERT_EQUAL(30, reader->acc_upper_bounds_[3]);
  }

  void test_size() {
    auto reader = create_case_1();
    reader->size(); // no meaning
  }

private:
  std::unique_ptr<WandReader<int, int>> create_case_1() {
    std::vector<std::unique_ptr<PostingListReader<int, int>>> readers;
    readers.emplace_back(new MockReader<int, int>({ // upper bound: 8
      {1, 8}, {2, 2}, {5, 4}, {8, 4}, {10, 2}
    }));
    readers.emplace_back(new MockReader<int, int>({ // upper bound: 2
      {1, 2}, {3, 1}, {5, 1}, {7, 2}, {8, 1}, {9, 2}
    }));
    readers.emplace_back(new MockReader<int, int>({ // upper bound 10
      {3, 1}, {5, 5}, {9, 10}, {10, 5}
    }));
    readers.emplace_back(new MockReader<int, int>({ // upper bound 10
      {5, 10}, {9, 10}
    }));
    return std::make_unique<WandReader<int, int>>(std::move(readers));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WandReaderTest);

} /* namespace redgiant */
