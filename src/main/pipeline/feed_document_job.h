#ifndef SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_
#define SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_

#include <ctime>

#include "../model/document.h"
#include "utils/stop_watch.h"

namespace redgiant {
struct FeedDocumentJob {
  // used for measuring feeding latency
  StopWatch watch;
  // doc feature
  Document doc;
  // expire time
  time_t expire_time;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_FEEDING_FEED_DOCUMENT_JOB_H_ */
