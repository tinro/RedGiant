#ifndef SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_
#define SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_

#include <memory>
#include "data/json_parser.h"

namespace redgiant {
class FeatureCache;
class FeatureVector;
class QueryRequest;

class QueryRequestParser: public JsonParser<QueryRequest> {
public:
  QueryRequestParser(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~QueryRequestParser() = default;

  virtual int parse_json(const rapidjson::Value &root, QueryRequest& output);

private:
  int parse_feature_spaces(const rapidjson::Value& json, QueryRequest& request);

  // parse feature vector contains multiple features and their weights.
  // e.g. { "favorite_sports" : { "football" : 1.0, "tennis" : 2.0 } }
  int parse_feature_vector_multiple_featuers(const rapidjson::Value& json,
      const QueryRequest& request, FeatureVector& vec);

private:
  std::shared_ptr<FeatureCache> cache_;
};

class QueryRequestParserFactory: public ParserFactory<QueryRequest> {
public:
  QueryRequestParserFactory(std::shared_ptr<FeatureCache> cache)
  : cache_(std::move(cache)) {
  }

  virtual ~QueryRequestParserFactory() = default;

  std::unique_ptr<Parser<QueryRequest>> create_parser() {
    return std::unique_ptr<Parser<QueryRequest>>(new QueryRequestParser(cache_));
  }

private:
  std::shared_ptr<FeatureCache> cache_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_QUERY_REQUEST_PARSER_H_ */
