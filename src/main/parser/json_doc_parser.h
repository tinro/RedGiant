#ifndef SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_
#define SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_

#include <memory>
#include <string>
#include <utility>

#include "model/doc_features.h"
#include "model/feature_cache.h"
#include "parser/json_parser.h"

namespace redgiant {
class JsonDocParser: public JsonParser<DocFeatures> {
public:
  JsonDocParser(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~JsonDocParser() = default;

  virtual std::unique_ptr<DocFeatures> parse_node(const rapidjson::Value &root);

private:
  std::shared_ptr<FeatureCache> cache_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_DATA_JSON_DOC_PARSER_H_ */
