#include "index/document_query.h"

#include <map>
#include <utility>
#include "core/query/dot_product_query.h"

namespace redgiant {

DocumentQuery::DocumentQuery(const QueryRequest& request) {
  typedef DotProductQuery<DocumentTraits::DocId, Score, const DocumentTraits::TermWeight&> ConcreteDocQuery;

  query_count_ = request.get_query_count();
  for (const auto& query_feature : request.get_query_features()) {
    doc_queries_.push_back(std::make_pair(query_feature.first,
        std::unique_ptr<ConcreteDocQuery>(new ConcreteDocQuery(query_feature.second))));
  }
}

} /* namespace redgiant */
