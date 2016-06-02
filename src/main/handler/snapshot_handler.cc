#include "handler/snapshot_handler.h"

#include <memory>
#include <utility>

#include "index/document_index_view.h"
#include "service/request_context.h"
#include "service/response_writer.h"
#include "utils/logger.h"
#include "utils/stop_watch.h"

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

void SnapshotHandler::handle_request(const RequestContext* request, ResponseWriter* response) {
  StopWatch watch;

  int method = request->get_method();
  if (method != RequestContext::METHOD_GET) {
    response->add_body("method should be GET\n");
    response->send(400, NULL);
    LOG_ERROR(logger, "method is not GET");
    return;
  }

  index_view_->dump(snapshot_prefix_);

  response->add_body(R"({"ret":"success"})" "\n");
  response->send(200, NULL);

  LOG_INFO(logger, "Dump snapshot completed, latency=%ldms", watch.get_ticks_ms());
}

} /* namespace redgiant */
