#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "data/document.h"
#include "data/document_parser.h"
#include "data/feature.h"
#include "data/feature_space.h"
#include "data/feature_space_manager.h"
#include "data/feature_vector.h"

using namespace std;

namespace redgiant {
class DocumentParserTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocumentParserTest);
  CPPUNIT_TEST(test_parser);
  CPPUNIT_TEST(test_parser_withoutuuid);
  CPPUNIT_TEST_SUITE_END();

public:
  DocumentParserTest() = default;
  virtual ~DocumentParserTest() = default;

protected:
  void test_parser() {
    auto feature_spaces = create_feature_spaces();
    auto parser = create_parser(feature_spaces);

    string s =
      R"({
        "uuid": "abcd1234-9876-1234-ffff-001122ddeeff",
        "features": {
          "publisher": "publisher_id_test",
          "time": "1234567",
          "score": 2.34,
          "category": ["1", "2", "3"], 
          "entity": {"ent_1":0.1, "ent_2":0.2, "ent_3" : 0.3},
          "unknown": "abc"
        }
      })";

    Document doc;
    int ret = parser->parse(s.c_str(), s.length(), doc);

    CPPUNIT_ASSERT_EQUAL(0, ret);
    // check doc meta
    CPPUNIT_ASSERT_EQUAL(string("abcd1234-9876-1234-ffff-001122ddeeff"), doc.get_id_str());
    CPPUNIT_ASSERT_EQUAL(string("abcd1234-9876-1234-ffff-001122ddeeff"), doc.get_id().to_string());

    const auto& vecs = doc.get_feature_vectors();
    CPPUNIT_ASSERT_EQUAL(5, (int)vecs.size());

    // feature vector 1: publisher
    auto fv = &(vecs[0]);
    CPPUNIT_ASSERT_EQUAL(string("publisher"), fv->get_space_name());
    CPPUNIT_ASSERT_EQUAL(string("string"), fv->get_space().get_type_name());
    // only one item
    CPPUNIT_ASSERT_EQUAL(1, (int)fv->get_features().size());
    auto f = &(fv->get_features()[0]);
    CPPUNIT_ASSERT_EQUAL(string("publisher_id_test"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)1.0, f->second, 0.0001);

    fv = &(vecs[1]);
    CPPUNIT_ASSERT_EQUAL(string("time"), fv->get_space_name());
    CPPUNIT_ASSERT_EQUAL(string("integer"), fv->get_space().get_type_name());
    // only one item
    CPPUNIT_ASSERT_EQUAL(1, (int)fv->get_features().size());
    f = &(fv->get_features()[0]);
    CPPUNIT_ASSERT_EQUAL(string("1234567"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)1.0, f->second, 0.0001);

    fv = &(vecs[2]);
    CPPUNIT_ASSERT_EQUAL(string("score"), fv->get_space_name());
    CPPUNIT_ASSERT_EQUAL(string("integer"), fv->get_space().get_type_name());
    // only one item
    CPPUNIT_ASSERT_EQUAL(1, (int)fv->get_features().size());
    f = &(fv->get_features()[0]);
    CPPUNIT_ASSERT_EQUAL(string("0"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)2.34, f->second, 0.0001);

    fv = &(vecs[3]);
    CPPUNIT_ASSERT_EQUAL(string("category"), fv->get_space_name());
    CPPUNIT_ASSERT_EQUAL(string("integer"), fv->get_space().get_type_name());
    // 3 items
    CPPUNIT_ASSERT_EQUAL(3, (int)fv->get_features().size());
    f = &(fv->get_features()[0]);
    CPPUNIT_ASSERT_EQUAL(string("1"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)1.0, f->second, 0.0001);
    f = &(fv->get_features()[1]);
    CPPUNIT_ASSERT_EQUAL(string("2"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)1.0, f->second, 0.0001);
    f = &(fv->get_features()[2]);
    CPPUNIT_ASSERT_EQUAL(string("3"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)1.0, f->second, 0.0001);

    fv = &(vecs[4]);
    CPPUNIT_ASSERT_EQUAL(string("entity"), fv->get_space_name());
    CPPUNIT_ASSERT_EQUAL(string("string"), fv->get_space().get_type_name());
    // 3 items
    CPPUNIT_ASSERT_EQUAL(3, (int)fv->get_features().size());
    f = &(fv->get_features()[0]);
    CPPUNIT_ASSERT_EQUAL(string("ent_1"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)0.1, f->second, 0.0001);
    f = &(fv->get_features()[1]);
    CPPUNIT_ASSERT_EQUAL(string("ent_2"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)0.2, f->second, 0.0001);
    f = &(fv->get_features()[2]);
    CPPUNIT_ASSERT_EQUAL(string("ent_3"), f->first->get_key());
    CPPUNIT_ASSERT_DOUBLES_EQUAL((double)0.3, f->second, 0.0001);
  }

  void test_parser_withoutuuid() {
    auto feature_spaces = create_feature_spaces();
    auto parser = create_parser(feature_spaces);

    string s =
      R"({
        "features": {
          "publisher": "publisher_id_test",
          "time": "1234567",
          "category": ["1", "2", "3"], 
          "entity": {"ent_1":0.1, "ent_2":0.2, "ent_3" : 0.3},
          "unknown": "abc"
        }
      })";

    Document doc;
    int ret = parser->parse(s.c_str(), s.length(), doc);
    CPPUNIT_ASSERT_EQUAL(-1, ret);
  }

private:
  std::shared_ptr<FeatureSpaceManager> create_feature_spaces() {
    auto feature_spaces = std::make_shared<FeatureSpaceManager>();
    feature_spaces->create_space("time", 1, FeatureSpace::SpaceType::kInteger);
    feature_spaces->create_space("publisher", 2, FeatureSpace::SpaceType::kString);
    feature_spaces->create_space("entity", 3, FeatureSpace::SpaceType::kString);
    feature_spaces->create_space("category", 4, FeatureSpace::SpaceType::kInteger);
    feature_spaces->create_space("score", 99, FeatureSpace::SpaceType::kInteger);
    return feature_spaces;
  };

  std::unique_ptr<DocumentParser> create_parser(std::shared_ptr<FeatureSpaceManager> feature_spaces) {
    return std::make_unique<DocumentParser>(std::move(feature_spaces));
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentParserTest);
} /* namespace redgiant */
