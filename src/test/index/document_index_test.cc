#include <fstream>
#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "index/document_index.h"

namespace redgiant {

class DocumentIndexTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DocumentIndexTest);
  CPPUNIT_TEST(test_dumper_and_loader);
  CPPUNIT_TEST_SUITE_END();

public:
  DocumentIndexTest() = default;
  virtual ~DocumentIndexTest() = default;

protected:
  typedef DocumentTraits::TermId TermId;
  typedef DocumentTraits::DocId DocId;
  typedef DocumentTraits::TermWeight TermWeight;
  typedef DocumentTraits::ExpireTime ExpireTime;

  void test_dumper_and_loader() {
    std::string snapshot_file_name = "test.snapshot.dump";
    BaseDocumentIndex<DocumentTraits> doc_idx_1(1, 1024);

    std::vector<TermId> term_set_1;
    term_set_1.push_back(11);
    term_set_1.push_back(12);

    std::vector<TermId> term_set_2;
    term_set_2.push_back(21);
    term_set_2.push_back(22);
    term_set_2.push_back(23);

    std::string id1 = "00000001-9876-1234-ffff-001122ddeeff";
    std::string id2 = "00000002-9876-1234-ffff-001122ddeeff";

    doc_idx_1.doc_term_map_[DocumentId(id1)] = std::move(term_set_1);
    doc_idx_1.doc_term_map_[DocumentId(id2)] = std::move(term_set_2);

    doc_idx_1.expire_.update(DocumentId(id2), 123456);
    doc_idx_1.expire_.update(DocumentId(id1), 123456);

    std::shared_ptr<SafePostingList<DocId, TermWeight>> plist(
        new SafePostingList<DocId, TermWeight>(
            std::make_shared<SafePostingListFactory<DocId, TermWeight>>(
                std::make_shared<BTreePostingListFactory<DocId, TermWeight>>())));

    doc_idx_1.index_[11] = plist;
    doc_idx_1.index_[12] = plist;
    doc_idx_1.index_[13] = plist;

    doc_idx_1.dump(SnapshotDumper(snapshot_file_name));

    BaseDocumentIndex<DocumentTraits> doc_idx_2(1, 1024, SnapshotLoader(snapshot_file_name));

    CPPUNIT_ASSERT_EQUAL(2, (int)doc_idx_2.doc_term_map_.size());
    CPPUNIT_ASSERT_EQUAL(doc_idx_1.doc_term_map_.size(), doc_idx_2.doc_term_map_.size());
    CPPUNIT_ASSERT_EQUAL(doc_idx_1.doc_term_map_[DocumentId(id1)].size(), doc_idx_2.doc_term_map_[DocumentId(id1)].size());
    CPPUNIT_ASSERT_EQUAL(doc_idx_1.doc_term_map_[DocumentId(id2)].size(), doc_idx_2.doc_term_map_[DocumentId(id2)].size());
    CPPUNIT_ASSERT_EQUAL(doc_idx_1.expire_.size(), doc_idx_2.expire_.size());
    CPPUNIT_ASSERT_EQUAL(doc_idx_1.index_.size(), doc_idx_2.index_.size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DocumentIndexTest);

} /* namespace redgiant */




