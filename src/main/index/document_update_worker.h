#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_

#include <memory>

#include "data/document_update_request.h"
#include "utils/concurrency/worker.h"

namespace redgiant {
class DocumentIndexManager;

class DocumentUpdateWorker: public Worker<DocumentUpdateRequest> {
public:
  DocumentUpdateWorker(DocumentIndexManager* index)
  : index_(index) {
  }

  virtual ~DocumentUpdateWorker() = default;

  virtual void prepare();
  virtual void cleanup();
  virtual void execute(DocumentUpdateRequest& job);

private:
  DocumentIndexManager* index_;
};

class FeedDocumentWorkerFactory: public WorkerFactory<DocumentUpdateWorker> {
public:
  FeedDocumentWorkerFactory(DocumentIndexManager* index)
  : index_(index) {
  }

  virtual ~FeedDocumentWorkerFactory() = default;

  virtual std::unique_ptr<DocumentUpdateWorker> create() {
    return std::unique_ptr<DocumentUpdateWorker>(new DocumentUpdateWorker(index_));
  }

private:
  DocumentIndexManager* index_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_ */
