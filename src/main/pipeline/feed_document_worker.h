#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_

#include <memory>

#include "feed_document_request.h"
#include "utils/concurrency/worker.h"

namespace redgiant {
class DocumentIndexManager;

class FeedDocumentWorker: public Worker<FeedDocumentRequest> {
public:
  FeedDocumentWorker(DocumentIndexManager* index)
  : index_(index) {
  }

  virtual ~FeedDocumentWorker() = default;

  virtual void prepare();

  virtual void cleanup();

  virtual void execute(FeedDocumentRequest& job);

private:
  DocumentIndexManager* index_;
};

class FeedDocumentWorkerFactory: public WorkerFactory<FeedDocumentWorker> {
public:
  FeedDocumentWorkerFactory(DocumentIndexManager* index)
  : index_(index) {
  }

  virtual ~FeedDocumentWorkerFactory() = default;

  virtual std::unique_ptr<FeedDocumentWorker> create() {
    return std::unique_ptr<FeedDocumentWorker>(new FeedDocumentWorker(index_));
  }

private:
  DocumentIndexManager* index_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_ */
