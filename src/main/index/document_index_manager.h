#ifndef SRC_MAIN_INDEX_DOCUMENT_INDEX_MANGER_H_
#define SRC_MAIN_INDEX_DOCUMENT_INDEX_MANGER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "model/document.h"
#include "index/document_index.h"
#include "index/document_query.h"
#include "index/index_manager.h"

namespace redgiant {
class QueryRequest;

class DocumentIndexManager: public IndexManager {
public:
  typedef DocumentIndex::DocId DocId;
  typedef DocumentIndex::TermId TermId;
  typedef DocumentIndex::TermWeight TermWeight;
  typedef DocumentIndex::TermPair TermPair;
  typedef DocumentIndex::DocTerms DocTerms;
  typedef DocumentIndex::DocTuple DocTuple;
  typedef DocumentIndex::RawReader RawReader;
  typedef DocumentQuery::Score Score;
  // the reader type is identical for both doc index and gmp index
  typedef DocumentIndex::Reader<Score> Reader;
  typedef DocumentIndex::ReaderPair<Score> ReaderPair;

  // create a default index.
  DocumentIndexManager(size_t doc_initial_buckets, size_t doc_max_size = 0);

  // recover an index from dumped snapshot.
  DocumentIndexManager(size_t doc_initial_buckets, size_t doc_max_size, const std::string& snapshot_prefix);

  virtual ~DocumentIndexManager() = default;

  const DocumentIndex& get_index() const {
    return index_;
  }

  virtual int do_maintain(time_t time);

  int dump(const std::string& snapshot_prefix);

  int remove(DocId doc_id);

  int batch_remove(const std::vector<DocId> doc_ids);

  int update(std::shared_ptr<Document> doc, time_t expire_time);

  int batch_update(const std::vector<std::shared_ptr<Document>>& docs, time_t expire_time);

  std::unique_ptr<RawReader> peek_term(TermId term_id) const;

//  std::shared_ptr<Document> peek_doc(DocId doc_id) const;

  std::unique_ptr<Reader> query(const QueryRequest& request, const DocumentQuery& query) const;

private:
  static const std::string kIndexFileNamePrefix;
  DocumentIndex index_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_INDEX_DOCUMENT_INDEX_MANGER_H_ */
