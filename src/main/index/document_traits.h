#ifndef SRC_MAIN_INDEX_DOCUMENT_TRAITS_H_
#define SRC_MAIN_INDEX_DOCUMENT_TRAITS_H_

#include <functional>

#include "data/document.h"
#include "data/document_id.h"
#include "data/feature.h"
#include "data/feature_vector.h"

namespace redgiant {
class DocumentTraits {
public:
  typedef Document Doc;
  typedef DocumentId DocId;
  typedef Feature::FeatureId TermId;
  typedef FeatureVector::FeatureWeight TermWeight;
  typedef int32_t ExpireTime;
  typedef DocumentId::Hash DocIdHash;
  typedef std::hash<TermId> TermIdHash;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_INDEX_DOCUMENT_TRAITS_H_ */
