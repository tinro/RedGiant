#include "index/document_update_worker.h"

#include <memory>

#include "data/document_update_request.h"
#include "index/document_update_worker.h"
#include "index/document_index_manager.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

void DocumentUpdateWorker::prepare() {
  LOG_DEBUG(logger, "worker started.");
}

void DocumentUpdateWorker::cleanup() {
}

void DocumentUpdateWorker::execute(DocumentUpdateRequest& job) {
  LOG_DEBUG(logger, "worker received job");
  index_->update(job.get_doc(), job.get_expire_time());
}

} /* namespace redgiant */
