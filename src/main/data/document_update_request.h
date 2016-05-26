#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_

#include <ctime>
#include <memory>
#include <utility>

#include "data/document.h"
#include "utils/stop_watch.h"

namespace redgiant {
class DocumentUpdateRequest {
public:
  DocumentUpdateRequest(std::shared_ptr<Document> doc, std::time_t expire_time,
      StopWatch watch = StopWatch())
  : doc_(std::move(doc)), expire_time_(expire_time), watch_(watch) {
  }

  const std::shared_ptr<Document>& get_doc() const {
    return doc_;
  }

  std::time_t get_expire_time() const {
    return expire_time_;
  }

  const StopWatch& get_watch() const {
    return watch_;
  }

private:
  std::shared_ptr<Document> doc_;
  std::time_t expire_time_;
  // used for measuring feeding latency
  StopWatch watch_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_ */
