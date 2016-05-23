#include "ranking/default_model.h"

#include <vector>
#include <utility>

#include "data/query_request.h"
#include "index/document_query.h"

namespace redgiant {

std::unique_ptr<DocumentQuery> DefaultModel::process(const QueryRequest& request) const {
  typedef DocumentQuery::Score Score;

  std::vector<DocumentQuery::TermPair> terms;
  for (const auto& fv: request.get_feature_vectors()) {
    for (const auto& f: fv.get_features()) {
      terms.emplace_back(f.first->get_id(), static_cast<Score>(f.second));
    }
  }
  return std::unique_ptr<DocumentQuery>(new DocumentQuery(request.get_query_count(), std::move(terms)));
}

} /* namespace redgiant */
