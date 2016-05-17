#ifndef SRC_MAIN_PARSER_DOCUMENT_PARSER_H_
#define SRC_MAIN_PARSER_DOCUMENT_PARSER_H_

#include <memory>
#include "parser/json_parser.h"

namespace redgiant {
class Document;
class FeatureCache;
class FeatureVector;

class DocumentParser: public JsonParser<Document> {
public:
  DocumentParser(std::shared_ptr<FeatureCache> cache)
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
  std::shared_ptr<FeatureCache> cache_;
};

class DocumentParserFactory: public ParserFactory<Document> {
public:
  DocumentParserFactory(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~DocumentParserFactory() = default;

  std::unique_ptr<Parser<Document>> create_parser() {
    return std::unique_ptr<Parser<Document>>(new DocumentParser(cache_));
  }

private:
  std::shared_ptr<FeatureCache> cache_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_PARSER_DOCUMENT_PARSER_H_ */
