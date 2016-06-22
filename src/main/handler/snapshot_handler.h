#ifndef SRC_MAIN_HANDLER_SNAPSHOT_HANDLER_H_
#define SRC_MAIN_HANDLER_SNAPSHOT_HANDLER_H_

#include <string>
#include <utility>

#include "service/request_handler.h"
#include "utils/cached_buffer.h"

namespace redgiant {

class DocumentIndexView;

class SnapshotHandler: public RequestHandler {
public:
  SnapshotHandler(DocumentIndexView* index_view, std::string snapshot_prefix)
  : index_view_(index_view), snapshot_prefix_(std::move(snapshot_prefix)) {
  }

  virtual ~SnapshotHandler() = default;

  virtual void handle_request(const RequestContext* request, ResponseWriter* response);

private:
  DocumentIndexView* index_view_;
  std::string snapshot_prefix_;
};

class SnapshotHandlerFactory: public RequestHandlerFactory {
public:
  SnapshotHandlerFactory(DocumentIndexView* index_view, std::string snapshot_prefix)
  : index_view_(index_view), snapshot_prefix_(std::move(snapshot_prefix)) {
  }

  virtual ~SnapshotHandlerFactory() = default;

  virtual std::unique_ptr<RequestHandler> create_handler() {
    return std::make_unique<SnapshotHandler>(index_view_, snapshot_prefix_);
  }

private:
  DocumentIndexView* index_view_;
  std::string snapshot_prefix_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_HANDLER_SNAPSHOT_HANDLER_H_ */
