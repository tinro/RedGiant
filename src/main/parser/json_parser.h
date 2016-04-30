#ifndef SRC_MAIN_DATA_JSON_PARSER_H_
#define SRC_MAIN_DATA_JSON_PARSER_H_

#include <memory>

#include "parser/parser.h"
#include "third_party/rapidjson/document.h"
#include "utils/logger.h"

namespace redgiant {
template <typename Output>
class JsonParser: public Parser<Output> {
public:
  JsonParser() = default;
  virtual ~JsonParser() = default;

  virtual std::unique_ptr<Output> parse(const char* str, size_t len) {
    DECLARE_LOGGER(logger, __FILE__);
    (void) len;
    rapidjson::Document root;
    if (root.Parse<0>(str).HasParseError()) {
      LOG_ERROR(logger, "parse error=%x, %zu", root.GetParseError(), root.GetErrorOffset());
      return nullptr;
    }
    return parse_json(root);
  }

  virtual std::unique_ptr<Output> parse_json(const rapidjson::Value &root) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_JSON_PARSER_H_ */
