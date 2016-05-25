#include <ctime>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "index/document_query.h"
#include "index/document_index.h"
#include "index/document_index_manager.h"

#include "data/document.h"
#include "data/feature.h"
#include "data/feature_vector.h"
#include "data/interm_query.h"
#include "data/query_request.h"
#include "utils/logger.h"

using namespace std;

namespace redgiant {
class DocumentIndexManagerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocumentIndexManagerTest);
  CPPUNIT_TEST(test_peek);
  CPPUNIT_TEST(test_exist_query);
  CPPUNIT_TEST(test_noexist_query);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_peek() {
    auto index = create_index_manager();

    CPPUNIT_ASSERT_EQUAL(12, (int)index->get_index().get_term_count());

    auto reader = index->peek_term(space_cat->create_feature("3")->get_id());
    //print_document_id(reader.get());

    auto cur_id = DocumentId(0);
    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0001-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.3, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0002-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0003-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(DocumentId(0).to_string(), cur_id.to_string());
  }

  void test_exist_query() {
    auto index = create_index_manager();
    QueryRequest request("ID-0001", 50, "", StopWatch(), true);
    DocumentQuery query(request, IntermQuery({
        {space_cat->calculate_feature_id("3"), 2.0},
        {space_ent->calculate_feature_id("AA"), 1.0},
        {space_ent->calculate_feature_id("zzz"), 5.0},
    }));

    auto reader = index->query(request, query);
    auto cur_id = DocumentId(0);
    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0001-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.7, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0002-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0003-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.4, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(string("00000000-0005-0000-0000-000000000000"), cur_id.to_string());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, reader->read(), 0.0001);

    cur_id = reader->next(cur_id);
    CPPUNIT_ASSERT_EQUAL(DocumentId(0).to_string(), cur_id.to_string());
  }

  void test_noexist_query() {
    auto index = create_index_manager();
    QueryRequest request("ID-0001", 50, "", StopWatch(), true);
    DocumentQuery query(request, IntermQuery({
        {space_cat->calculate_feature_id("5"), 2.0},
        {space_ent->calculate_feature_id("ooo"), 1.0},
    }));

    auto reader = index->query(request, query);
    CPPUNIT_ASSERT(!reader);
  }

private:
  std::shared_ptr<FeatureSpace> space_cat =
      std::make_shared<FeatureSpace>("categories", 1, FeatureSpace::SpaceType::kInteger);
  std::shared_ptr<FeatureSpace> space_ent =
      std::make_shared<FeatureSpace>("entities", 2, FeatureSpace::SpaceType::kString);

  std::shared_ptr<Document> create_document(std::string id,
      std::vector<std::pair<std::shared_ptr<FeatureSpace>, std::vector<std::pair<std::string, double>>>> features) {
    auto doc = std::make_shared<Document>(std::move(id));
    for (auto& s: features) {
      FeatureVector vec(std::move(s.first));
      for (auto& p: s.second) {
        vec.add_feature(p.first, p.second);
      }
      doc->add_feature_vector(std::move(vec));
    }
    return doc;
  }

  std::unique_ptr<DocumentIndexManager> create_index_manager() {
    auto index = std::unique_ptr<DocumentIndexManager>(new DocumentIndexManager(1000, 1000));
    // create document vectors
    index->update(create_document(
        "00000000-0001-0000-0000-000000000000",
        {
          { space_cat, {{"1", 1.1}, {"2", 2.2}, {"3", 3.3}}},
          { space_ent, {{"AA", 0.1}, {"BB", 0.2}}},
        }), 1);

    index->update(create_document(
        "00000000-0003-0000-0000-000000000000",
        {
          { space_cat, {{"1", 1.5}, {"3", 0.2}}},
          { space_ent, {{"AA", 1.0}}},
        }), 1);

    index->update(create_document(
        "00000000-0004-0000-0000-000000000000",
        {
          { space_cat, {{"1001", 1.0}, {"1002", 1.5}}},
          { space_ent, {{"xxx", 0.5}, {"yyy", 0.5}}},
        }), 1);

    index->update(create_document(
        "00000000-0005-0000-0000-000000000000",
        {
          { space_cat, {{"1002", 2.0}, {"1003", 0.3}}},
          { space_ent, {{"xxx", 0.5}, {"zzz", 0.5}}},
        }), 1);

    index->update(create_document(
        "00000000-0002-0000-0000-000000000000",
        {
          { space_cat, {{"3", 1.5}, {"2", 1.4}}},
          { space_ent, {{"BB", 0.3}, {"CC", 1.0}}},
        }), 1);

    index->do_maintain(0);
    return index;
  }

  void print_document_id(DocumentIndex::RawReader* reader) {
    DocumentIndex::DocId doc_id;
    for (DocumentIndex::DocId iter = reader->next(doc_id);; iter = reader->next(iter)) {
      if (iter == doc_id)
        break;
      cout << "Doc id: " << iter.to_string() << endl;
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentIndexManagerTest);
}
