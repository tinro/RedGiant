#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_

#include <memory>
#include "pipeline/feed_document_job.h"
#include "utils/concurrency/worker.h"

namespace redgiant {
class FeedDocumentWorker: public Worker<FeedDocumentJob> {
public:
  FeedDocumentWorker();

  virtual ~FeedDocumentWorker() = default;

  virtual void prepare();

  virtual void cleanup();

  virtual void execute(FeedDocumentJob& job);

private:
};

class FeedDocumentWorkerFactory: public WorkerFactory<FeedDocumentWorker> {
public:
  FeedDocumentWorkerFactory() {
  }

  virtual ~FeedDocumentWorkerFactory() = default;

  virtual std::unique_ptr<FeedDocumentWorker> create() {
    return std::unique_ptr<FeedDocumentWorker>(new FeedDocumentWorker());
  }

private:
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_WORKER_H_ */
