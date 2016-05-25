#include "ranking/default_model.h"

#include <vector>
#include <utility>

#include "data/query_request.h"
#include "index/document_query.h"

namespace redgiant {

std::unique_ptr<IntermQuery> DefaultModel::process(const QueryRequest& request) const {
  IntermQuery::QueryFeatures terms;
  for (const auto& fv: request.get_feature_vectors()) {
    for (const auto& f: fv.get_features()) {
      terms.emplace_back(f.first->get_id(), static_cast<IntermQuery::QueryWeight>(f.second));
    }
  }
  return std::unique_ptr<IntermQuery>(new IntermQuery(std::move(terms)));
}

} /* namespace redgiant */
