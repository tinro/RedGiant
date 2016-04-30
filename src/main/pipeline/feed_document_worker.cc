#include "pipeline/feed_document_worker.h"

#include "utils/logger.h"
#include "utils/stop_watch.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

FeedDocumentWorker::FeedDocumentWorker() {
}

void FeedDocumentWorker::prepare() {
  LOG_DEBUG(logger, "worker started.");
}

void FeedDocumentWorker::cleanup() {
}

void FeedDocumentWorker::execute(FeedDocumentJob& job) {
  LOG_DEBUG(logger, "worker received job");
}

} /* namespace redgiant */
