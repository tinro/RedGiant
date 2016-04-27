#ifndef SRC_MAIN_REDGIANT_CORE_READER_DOT_PRODUCT_READER_H_
#define SRC_MAIN_REDGIANT_CORE_READER_DOT_PRODUCT_READER_H_

#include <memory>
#include <type_traits>
#include "core/reader/algorithms.h"
#include "core/reader/posting_list_reader.h"

namespace redgiant {
// InputWeight may be reference, QueryWeight is considered as value type
template <typename DocId, typename Score, typename InputWeight,
    typename QueryWeight = typename std::decay<InputWeight>::type,
    typename ScoreCombiner = DotProduct<Score, typename std::decay<InputWeight>::type, QueryWeight>>
class DotProductReader: public PostingListReader<DocId, Score> {
public:
  typedef PostingListReader<DocId, InputWeight> InputReader;

  DotProductReader(std::unique_ptr<InputReader> reader, const QueryWeight& query)
  : reader_(std::move(reader)), query_(query), combiner_() {
  }

  DotProductReader(std::unique_ptr<InputReader> reader, const QueryWeight& query, const ScoreCombiner& combiner)
  : reader_(std::move(reader)), query_(query), combiner_(combiner) {
  }

  virtual ~DotProductReader() = default;

  virtual DocId next(DocId current) {
    return reader_->next(current);
  }

  virtual Score read() {
    return combiner_(reader_->read(), query_);
  }

  virtual Score upper_bound() {
    return combiner_(reader_->upper_bound(), query_);
  }

  virtual size_t size() const {
    return reader_->size();
  }

private:
  std::unique_ptr<InputReader> reader_;
  QueryWeight query_;
  ScoreCombiner combiner_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_CORE_READER_DOT_PRODUCT_READER_H_ */
