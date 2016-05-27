#include "index/document_update_pipeline.h"

#include <memory>
#include <utility>

#include "data/document_update_request.h"
#include "index/document_update_worker.h"
#include "utils/logger.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

DocumentUpdatePipeline::DocumentUpdatePipeline(size_t thread_num,
    size_t queue_size, DocumentIndexManager* index) {
  feed_document_ = std::make_shared<WorkerExecutor<DocumentUpdateRequest, DocumentUpdateWorker>>(
      std::make_shared<FeedDocumentWorkerFactory>(index), thread_num, queue_size);
}

void DocumentUpdatePipeline::start() {
  feed_document_->start();
}

void DocumentUpdatePipeline::stop() {
  feed_document_->stop();
}

void DocumentUpdatePipeline::schedule(std::shared_ptr<DocumentUpdateRequest> job) {
  feed_document_->schedule(std::move(job));
  LOG_TRACE(logger, "job pushed, queue size: %zu", feed_document_->get_queue_size());
}

} /* namespace redgiant */
