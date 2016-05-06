#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "core/index/btree_expire_table.h"

namespace redgiant {
class BTreeExpireTableTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BTreeExpireTableTest);
  CPPUNIT_TEST(test_empty);
  CPPUNIT_TEST(test_size_update);
  CPPUNIT_TEST(test_size_remove);
  CPPUNIT_TEST(test_expire);
  CPPUNIT_TEST(test_expire_with_limit);
  CPPUNIT_TEST(test_force_expire);
  CPPUNIT_TEST_SUITE_END();

public:
  BTreeExpireTableTest() = default;
  virtual ~BTreeExpireTableTest() = default;

protected:
  void test_empty() {
    bool empty;
    auto et = create_case_empty();
    empty = et->empty();
    CPPUNIT_ASSERT_EQUAL(true, empty);

    int ret;
    ret = et->update(1, 1);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    empty = et->empty();
    CPPUNIT_ASSERT_EQUAL(false, empty);
  }

  void test_size_update() {
    int size;
    auto et = create_case_empty();
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(0, size);

    int ret;
    // update success
    ret = et->update(1, 1);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    et->update(2, 1);
    et->update(3, 1);
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(3, size);
  }

  void test_size_remove() {
    int size;
    auto et = create_case_empty();
    et->update(1, 1);
    et->update(2, 1);
    et->update(3, 1);
    et->update(4, 1);
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(4, size);

    int ret;
    // remove success
    ret = et->remove(1);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(3, size);

    // remove success
    ret = et->remove(3);
    CPPUNIT_ASSERT_EQUAL(1, ret);
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(2, size);

    // remove failed
    ret = et->remove(5);
    CPPUNIT_ASSERT_EQUAL(0, ret);
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(2, size);
  }

  void test_expire() {
    auto et = create_case_empty();
    fill_case_1(*et);

    std::vector<std::pair<int, int>> vec;
    int size;
    // expire at 1002
    vec = et->expire(1002);
    CPPUNIT_ASSERT_EQUAL(2, (int)vec.size());
    CPPUNIT_ASSERT(any_of_key(vec, 1));
    CPPUNIT_ASSERT(any_of_key(vec, 10));
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(8, size);

    // expire at 1005
    vec = et->expire(1005);
    CPPUNIT_ASSERT_EQUAL(3, (int)vec.size());
    CPPUNIT_ASSERT(any_of_key(vec, 2));
    CPPUNIT_ASSERT(any_of_key(vec, 9));
    CPPUNIT_ASSERT(any_of_key(vec, 3));
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(5, size);

    // will return empty
    vec = et->expire(1004);
    CPPUNIT_ASSERT_EQUAL(0, (int)vec.size());
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(5, size);
  }

  void test_expire_with_limit() {
    auto et = create_case_empty();
    fill_case_1(*et);

    std::vector<std::pair<int, int>> vec;
    int size;
    // expire at time 1003, while limit expire_table size to 5
    vec = et->expire_with_limit(1003, 5);
    CPPUNIT_ASSERT_EQUAL(5, (int)vec.size());
    CPPUNIT_ASSERT(any_of_key(vec, 1));
    CPPUNIT_ASSERT(any_of_key(vec, 10));
    CPPUNIT_ASSERT(any_of_key(vec, 2));
    CPPUNIT_ASSERT(any_of_key(vec, 9));
    CPPUNIT_ASSERT(any_of_key(vec, 3));
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(5, size);

    // expire at time 1003, while limit expire_table size to 10
    vec = et->expire_with_limit(1006, 10);
    CPPUNIT_ASSERT_EQUAL(1, (int)vec.size());
    CPPUNIT_ASSERT(any_of_key(vec, 8));
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(4, size);

    // will return empty
    vec = et->expire_with_limit(1004, 10);
    CPPUNIT_ASSERT_EQUAL(0, (int)vec.size());
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(4, size);
  }

  void test_force_expire() {
    auto et = create_case_empty();
    fill_case_1(*et);

    std::vector<std::pair<int, int>> vec;
    int size;
    // limit expire_table size to 6
    vec = et->force_expire(6);
    CPPUNIT_ASSERT_EQUAL(4, (int)vec.size());
    CPPUNIT_ASSERT(any_of_key(vec, 1));
    CPPUNIT_ASSERT(any_of_key(vec, 10));
    CPPUNIT_ASSERT(any_of_key(vec, 2));
    CPPUNIT_ASSERT(any_of_key(vec, 9));
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(6, size);

    // will return empty
    vec = et->force_expire(10);
    CPPUNIT_ASSERT_EQUAL(0, (int)vec.size());
    size = et->size();
    CPPUNIT_ASSERT_EQUAL(6, size);
  }

  virtual std::unique_ptr<BTreeExpireTable<int, int>> create_case_empty() {
    return std::unique_ptr<BTreeExpireTable<int, int>>(new BTreeExpireTable<int, int>());
  }

  template <typename K, typename V>
  static bool any_of_key(const std::vector<std::pair<K, V>>& vec, const K& key) {
    return std::any_of(vec.begin(), vec.end(), [key] (const std::pair<K, V>& p) { return p.first == key; });
  }

private:
  void fill_case_1(BTreeExpireTable<int, int>& expire_table) {
    expire_table.update(1, 1001);
    expire_table.update(2, 1003);
    expire_table.update(3, 1005);
    expire_table.update(4, 1007);
    expire_table.update(5, 1009);
    expire_table.update(6, 1010);
    expire_table.update(7, 1008);
    expire_table.update(8, 1006);
    expire_table.update(9, 1004);
    expire_table.update(10,1002);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BTreeExpireTableTest);

} /* namespace redgiant */
