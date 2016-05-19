#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_

#include <memory>

#include "feed_document_request.h"
#include "pipeline/feed_document_worker.h"
#include "utils/concurrency/job_executor.h"
#include "utils/concurrency/worker_executor.h"

namespace redgiant {
class DocumentIndexManager;

class FeedDocumentPipeline: public JobExecutor<FeedDocumentRequest> {
public:
  FeedDocumentPipeline(size_t thread_num, size_t queue_size, DocumentIndexManager* index);
  virtual ~FeedDocumentPipeline() = default;

  virtual void start();
  virtual void stop();
  virtual void schedule(std::shared_ptr<FeedDocumentRequest> job);

private:
  std::shared_ptr<WorkerExecutor<FeedDocumentRequest, FeedDocumentWorker>> feed_document_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_ */
