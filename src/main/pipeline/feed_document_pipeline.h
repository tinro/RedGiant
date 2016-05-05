#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_

#include "pipeline/feed_document_job.h"
#include "pipeline/feed_document_worker.h"
#include "utils/concurrency/job_executor.h"
#include "utils/concurrency/worker_executor.h"

namespace redgiant {
class FeedDocumentPipeline: public JobExecutor<FeedDocumentJob> {
public:
  FeedDocumentPipeline(size_t thread_num, size_t queue_size);
  virtual ~FeedDocumentPipeline() = default;

  virtual void start();
  virtual void stop();
  virtual void schedule(std::shared_ptr<FeedDocumentJob> job);

private:
  std::shared_ptr<WorkerExecutor<FeedDocumentJob, FeedDocumentWorker>> feed_document_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_ */
