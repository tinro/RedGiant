#ifndef SRC_MAIN_WRAPPER_DOCUMENT_INDEX_WRAPPER_H_
#define SRC_MAIN_WRAPPER_DOCUMENT_INDEX_WRAPPER_H_

#include <memory>
#include <string>

#include "utils/concurrency/job_executor.h"

namespace redgiant {
class Document;
class DocumentIndexManager;
class DocumentUpdateRequest;

class DocumentIndexView {
public:
  DocumentIndexView(DocumentIndexManager* index, JobExecutor<DocumentUpdateRequest>* pipeline);
  ~DocumentIndexView() = default;

  void update_document(const std::string& uuid, time_t expire_time, std::shared_ptr<Document> doc);

  void update_document_async(const std::string& uuid, time_t expire_time, std::shared_ptr<Document> doc);

  void remove_document(const std::string& uuid);

  //void remove_document_async(const std::string& uuid);

  //std::shared_ptr<Document> peek_document(const std::string& uuid);

  void dump(const std::string& snapshot_prefix);

private:
  DocumentIndexManager* index_;
  JobExecutor<DocumentUpdateRequest>* update_pipeline_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_WRAPPER_DOCUMENT_INDEX_WRAPPER_H_ */
