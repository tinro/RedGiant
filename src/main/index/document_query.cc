#include "index/document_query.h"

#include <map>
#include <utility>
#include "core/query/dot_product_query.h"

namespace redgiant {

DocumentQuery::DocumentQuery(size_t query_count, const std::vector<TermPair>& query_terms)
: query_count_(query_count){
  typedef DotProductQuery<DocumentTraits::DocId, Score, const DocumentTraits::TermWeight&> ConcreteDocQuery;

  for (const auto& term_pair : query_terms) {
    doc_queries_.push_back(std::make_pair(term_pair.first,
        std::unique_ptr<ConcreteDocQuery>(new ConcreteDocQuery(term_pair.second))));
  }
}

} /* namespace redgiant */
