#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_

#include <memory>

#include "utils/concurrency/job_executor.h"
#include "utils/concurrency/worker_executor.h"

namespace redgiant {
class DocumentIndexManager;
class DocumentUpdateRequest;
class DocumentUpdateWorker;

class DocumentUpdatePipeline: public JobExecutor<DocumentUpdateRequest> {
public:
  DocumentUpdatePipeline(size_t thread_num, size_t queue_size, DocumentIndexManager* index);
  virtual ~DocumentUpdatePipeline() = default;

  virtual void start();
  virtual void stop();
  virtual void schedule(std::shared_ptr<DocumentUpdateRequest> job);

private:
  std::shared_ptr<WorkerExecutor<DocumentUpdateRequest, DocumentUpdateWorker>> feed_document_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_PIPELINE_H_ */
