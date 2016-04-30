#ifndef SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_
#define SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "model/doc_features.h"
#include "parser/json_parser.h"

namespace redgiant {
class FeatureSpace;
class FeatureCache;

class JsonDocParser: public JsonParser<DocFeatures> {
public:
  JsonDocParser(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~JsonDocParser() = default;

  virtual std::unique_ptr<DocFeatures> parse_json(const rapidjson::Value &root);

private:
  void parse_feature_spaces(const rapidjson::Value& root, DocFeatures& doc);

  void parse_single_value_feature_space(const rapidjson::Value& root, const DocFeatures& doc,
      const std::shared_ptr<FeatureSpace>& space, DocFeatures::FeatureWeights& features);

  void parse_multi_value_feature_space(const rapidjson::Value& root, const DocFeatures& doc,
      const std::shared_ptr<FeatureSpace>& space, DocFeatures::FeatureWeights& features);

  std::shared_ptr<FeatureCache> cache_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_ */
