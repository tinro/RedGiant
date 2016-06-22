#ifndef SRC_MAIN_CORE_QUERY_DOT_PRODUCT_QUERY_H_
#define SRC_MAIN_CORE_QUERY_DOT_PRODUCT_QUERY_H_

#include <memory>
#include <type_traits>
#include <utility>
#include "core/query/posting_list_query.h"
#include "core/reader/dot_product_reader.h"

namespace redgiant {
// InputWeight may be reference, QueryWeight is considered as value type
template<typename DocId, typename Score, typename InputWeight,
    typename QueryWeight = typename std::decay<InputWeight>::type,
    typename ScoreCombiner = DotProduct<Score, typename std::decay<InputWeight>::type, QueryWeight>>
class DotProductQuery: public PostingListQuery<DocId, Score, InputWeight>{
public:
  typedef PostingListQuery<DocId, Score, InputWeight> Base;
  typedef DotProductReader<DocId, Score, InputWeight, QueryWeight, ScoreCombiner> ScoreReader;
  typedef typename Base::Reader Reader;
  typedef typename Base::InputReader InputReader;

  DotProductQuery(const QueryWeight& query)
  : query_(query), combiner_() {
  }

  DotProductQuery(const QueryWeight& query, ScoreCombiner combiner)
  : query_(query), combiner_(combiner) {
  }

  virtual ~DotProductQuery() = default;

  virtual std::unique_ptr<Reader> query(std::unique_ptr<InputReader> reader) const {
    return std::make_unique<ScoreReader>(std::move(reader), query_, combiner_);
  }

  const QueryWeight & get_query_weight() const {
    return query_;
  }

private:
  QueryWeight query_;
  ScoreCombiner combiner_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_QUERY_DOT_PRODUCT_QUERY_H_ */
