#include "index/document_query.h"

#include <map>
#include <utility>

#include "core/query/dot_product_query.h"
#include "data/interm_query.h"
#include "data/query_request.h"

namespace redgiant {

DocumentQuery::DocumentQuery(const QueryRequest& request, const IntermQuery& interm_query)
: query_count_(request.get_query_count()){
  typedef DotProductQuery<DocumentTraits::DocId, Score, const IntermQuery::QueryWeight&> ConcreteDocQuery;

  for (const auto& term_pair : interm_query.get_features()) {
    doc_queries_.push_back(std::make_pair(term_pair.first,
        std::unique_ptr<ConcreteDocQuery>(new ConcreteDocQuery(term_pair.second))));
  }
}

} /* namespace redgiant */
