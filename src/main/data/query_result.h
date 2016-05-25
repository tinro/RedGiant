#ifndef SRC_MAIN_DATA_QUERY_RESULT_H_
#define SRC_MAIN_DATA_QUERY_RESULT_H_

#include <string>

#include "utils/latency_tracker.h"
#include "utils/stop_watch.h"

namespace redgiant {
class QueryResult {
public:
  typedef double Score;
  typedef std::pair<std::string, Score> KeyScorePair;
  typedef std::vector<KeyScorePair> KeyScores;

  enum TimePhase {
    kStart = 0,
    kLoadModel,
    kBuildQuery,
    kQueryStart,
    kQueryExecute,
    kQueryRead,
    kResultConvert,
    kFinalize,

    kTimePhaseCount
  };

  QueryResult(const std::string& request_id)
  : request_id_(request_id) {
  }

  // no copy
  QueryResult(const QueryResult&) = delete;
  QueryResult& operator= (const QueryResult&) = delete;

  // movable
  QueryResult(QueryResult&&) = default;
  QueryResult& operator= (QueryResult&&) = default;

  ~QueryResult() = default;

  const std::string& get_request_id() const {
    return request_id_;
  }

  KeyScores& get_results() {
    return results_;
  }

  const KeyScores& get_results() const {
    return results_;
  }

  void track_latency(const StopWatch& watch, int i) {
    latency_tracker_.tick(watch, i);
  }

  // note: do boundary check in the caller
  long get_latency(int i) const {
    return latency_tracker_.get(i);
  }

private:
  std::string request_id_;
  KeyScores results_;
  LatencyTracker<kTimePhaseCount> latency_tracker_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_QUERY_RESULT_H_ */
