#ifndef SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_
#define SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_

#include <memory>
#include "data/json_parser.h"

namespace redgiant {
class FeatureSpaceManager;
class FeatureVector;
class QueryRequest;

class QueryRequestParser: public JsonParser<QueryRequest> {
public:
  QueryRequestParser(std::shared_ptr<FeatureSpaceManager> feature_spaces)
  : feature_spaces_(std::move(feature_spaces)) {
  }

  virtual ~QueryRequestParser() = default;

  virtual int parse_json(const rapidjson::Value &root, QueryRequest& output);

private:
  int parse_feature_spaces(const rapidjson::Value& json, QueryRequest& request);

  // parse feature vector contains only one single feature that is a weight.
  // e.g. { "download_count" : 3.0 }
  int parse_feature_vector_single_weighted(const rapidjson::Value& json,
      const QueryRequest& request, FeatureVector& vec);

  // parse feature vector contains only one single feature with no weight.
  // e.g. { "publisher" : "cnn" }
  int parse_feature_vector_single_unitary(const rapidjson::Value& json,
      const QueryRequest& request, FeatureVector& vec);

  // parse feature vector contains multiple features and their weights.
  // e.g. { "favorite_sports" : [ "football", "tennis" ] }
  int parse_feature_vector_multiple_unitary(const rapidjson::Value& json,
      const QueryRequest& request, FeatureVector& vec);

  // parse feature vector contains multiple features and their weights.
  // e.g. { "favorite_sports" : { "football" : 1.0, "tennis" : 2.0 } }
  int parse_feature_vector_multiple_weighted(const rapidjson::Value& json,
      const QueryRequest& request, FeatureVector& vec);

private:
  std::shared_ptr<FeatureSpaceManager> feature_spaces_;
};

class QueryRequestParserFactory: public ParserFactory<QueryRequest> {
public:
  QueryRequestParserFactory(std::shared_ptr<FeatureSpaceManager> feature_spaces)
  : feature_spaces_(std::move(feature_spaces)) {
  }

  virtual ~QueryRequestParserFactory() = default;

  std::unique_ptr<Parser<QueryRequest>> create_parser() {
    return std::make_unique<QueryRequestParser>(feature_spaces_);
  }

private:
  std::shared_ptr<FeatureSpaceManager> feature_spaces_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_ */
