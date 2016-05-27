#ifndef SRC_MAIN_DATA_DOCUMENT_PARSER_H_
#define SRC_MAIN_DATA_DOCUMENT_PARSER_H_

#include <memory>
#include "data/json_parser.h"

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
  int parse_feature_spaces(const rapidjson::Value& json, Document& doc);

  // parse feature vector contains only one single feature with no weight.
  // e.g. { "publisher" : "cnn" }
  int parse_feature_vector_single_feature(const rapidjson::Value& json,
      const Document& doc, FeatureVector& vec);

  // parse feature vector contains only one single feature that is a weight.
  // e.g. { "download_count" : 123456 }
  int parse_feature_vector_single_score(const rapidjson::Value& json,
      const Document& doc, FeatureVector& vec);

  // parse feature vector contains multiple features and their weights.
  // e.g. { "favorite_sports" : { "football" : 1.0, "tennis" : 2.0 } }
  int parse_feature_vector_multiple_featuers(const rapidjson::Value& json,
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

#endif /* SRC_MAIN_DATA_DOCUMENT_PARSER_H_ */
