#ifndef SRC_MAIN_PARSER_JSON_PARSER_H_
#define SRC_MAIN_PARSER_JSON_PARSER_H_

#include "parser/parser.h"
#include "third_party/rapidjson/document.h"

namespace redgiant {
template <typename Output>
class JsonParser: public Parser<Output> {
public:
  JsonParser() = default;
  virtual ~JsonParser() = default;

  virtual int parse(const char* str, size_t len, Output& output) {
    (void) len;
    rapidjson::Document root;
    if (root.Parse(str).HasParseError()) {
      return -1;
    }
    return parse_json(root, output);
  }

  virtual int parse_json(const rapidjson::Value &root, Output& output) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_PARSER_JSON_PARSER_H_ */
