#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "model/document_id.h"

#include <iostream>

using namespace std;

namespace redgiant {
class DocumentIdTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocumentIdTest);
  CPPUNIT_TEST(test_doc_id_constructor);
  CPPUNIT_TEST(test_doc_id_operation);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_doc_id_constructor() {
    string uuid = "a3269eb7-0de1-337d-8fa1-b929eb92b5ff";
    DocumentId document_id(uuid);
    string new_uuid(document_id.to_string());
    CPPUNIT_ASSERT_EQUAL(uuid,  new_uuid);

    string uuid_default = "00000000-0000-0000-0000-000000000000";
    DocumentId document_id_default;
    string new_uuid_default(document_id_default.to_string());
    CPPUNIT_ASSERT_EQUAL(uuid_default,  new_uuid_default);

    string uuid_param = "0000000c-0000-0000-2200-000000000000";
    DocumentId document_id_param(12, 34);
    string new_uuid_param(document_id_param.to_string());
    CPPUNIT_ASSERT_EQUAL(uuid_param, new_uuid_param);
  }

  void test_doc_id_operation() {
    DocumentId document_id_1(0x12, 0x34);
    DocumentId document_id_2(0x12, 0x34);
    CPPUNIT_ASSERT(document_id_1 == document_id_2);
    CPPUNIT_ASSERT(document_id_1 <= document_id_2);
    CPPUNIT_ASSERT(document_id_1 >= document_id_2);
    CPPUNIT_ASSERT(!(document_id_1 != document_id_2));

    DocumentId document_id_3(0x12, 0x34);
    DocumentId document_id_4(0x12, 0x35);
    CPPUNIT_ASSERT(document_id_3 < document_id_4);
    CPPUNIT_ASSERT(document_id_4 > document_id_3);

    DocumentId document_id_5;
    DocumentId document_id_6(1);
    if (document_id_5) {
      CPPUNIT_ASSERT(false);
    } else {
      CPPUNIT_ASSERT(true);
    }

    if (document_id_6) {
      CPPUNIT_ASSERT(true);
    } else {
      CPPUNIT_ASSERT(false);
    }

    DocumentId document_id_7(0x13, 0x34);
    DocumentId document_id_8(0x12, 0x34);
    CPPUNIT_ASSERT(document_id_8 < document_id_7);
    CPPUNIT_ASSERT(document_id_7 > document_id_8);

    DocumentId document_id_9(0xffffffffffffffff);
    DocumentId document_id_10(0x0,0x1);
    ++ document_id_9;
    CPPUNIT_ASSERT_EQUAL(document_id_9, document_id_10);
    -- document_id_10;
    CPPUNIT_ASSERT_EQUAL(document_id_10, DocumentId(0xffffffffffffffff));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentIdTest);
}
