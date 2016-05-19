#include "pipeline/feed_document_worker.h"

#include <memory>
#include "index/document_index_manager.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

void FeedDocumentWorker::prepare() {
  LOG_DEBUG(logger, "worker started.");
}

void FeedDocumentWorker::cleanup() {
}

void FeedDocumentWorker::execute(FeedDocumentRequest& job) {
  LOG_DEBUG(logger, "worker received job");
  index_->update(job.get_doc(), job.get_expire_time());
}

} /* namespace redgiant */
