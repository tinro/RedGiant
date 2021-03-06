#ifndef SRC_MAIN_DATA_DOCUMENT_H_
#define SRC_MAIN_DATA_DOCUMENT_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data/document_id.h"
#include "data/feature.h"
#include "data/feature_space.h"
#include "data/feature_vector.h"

namespace redgiant {

class Document {
public:
  Document() = default;

  Document(std::string id)
  : id_(id) {
    id_str_ = std::move(id);
  }

  Document(const Document&) = default;
  Document(Document&&) = default;
  ~Document() = default;

  const DocumentId& get_id() const {
    return id_;
  }

  const std::string& get_id_str() const {
    return id_str_;
  }

  void set_doc_id(std::string id) {
    id_ = DocumentId(id);
    id_str_ = std::move(id);
  }

  const std::vector<FeatureVector>& get_feature_vectors() const {
    return feature_vectors_;
  }

  void add_feature_vector(FeatureVector feature_vector) {
    feature_vectors_.push_back(std::move(feature_vector));
  }

private:
  DocumentId id_;
  std::string id_str_;
  std::vector<FeatureVector> feature_vectors_;
};

} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_DOCUMENT_H_ */
