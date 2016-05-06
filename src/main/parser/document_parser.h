#ifndef SRC_MAIN_PARSER_DOCUMENT_PARSER_H_
#define SRC_MAIN_PARSER_DOCUMENT_PARSER_H_

#include <memory>
#include <utility>

#include "model/document.h"
#include "parser/json_parser.h"

namespace redgiant {
class FeatureSpace;
class FeatureCache;

class DocumentParser: public JsonParser<Document> {
public:
  DocumentParser(FeatureCache* cache)
  : cache_(std::move(cache)) {
  }

  virtual ~DocumentParser() = default;

  virtual int parse_json(const rapidjson::Value &root, Document& output);

private:
  int parse_feature_spaces(const rapidjson::Value& root, Document& doc);

  int parse_single_value_feature_vector(const rapidjson::Value& root,
      const Document& doc, FeatureVector& vec);

  int parse_multi_value_feature_vector(const rapidjson::Value& root,
      const Document& doc, FeatureVector& vec);

private:
  FeatureCache* cache_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_PARSER_DOCUMENT_PARSER_H_ */
