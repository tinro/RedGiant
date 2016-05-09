#ifndef SRC_MAIN_PARSER_FEATURE_CACHE_PARSER_H_
#define SRC_MAIN_PARSER_FEATURE_CACHE_PARSER_H_

#include "model/feature_cache.h"
#include "parser/json_parser.h"

namespace redgiant {

class FeatureCacheParser: public JsonParser<FeatureCache> {
public:
  FeatureCacheParser() = default;
  virtual ~FeatureCacheParser() = default;

  virtual int parse_json(const rapidjson::Value &root, FeatureCache& output);
};

} /* namespace redgiant */

#endif /* SRC_MAIN_PARSER_FEATURE_CACHE_PARSER_H_ */
