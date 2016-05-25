#ifndef SRC_MAIN_DATA_JSON_PARSER_H_
#define SRC_MAIN_DATA_JSON_PARSER_H_

#include <cstdio>
#include "data/parser.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/filereadstream.h"

namespace redgiant {
template <typename Output>
class JsonParser: public Parser<Output> {
public:
  JsonParser() = default;
  virtual ~JsonParser() = default;

  virtual int parse(const char* str, size_t len, Output& output) {
    rapidjson::Document root;
    if (root.Parse(str, len).HasParseError()) {
      return -1;
    }
    return parse_json(root, output);
  }

  virtual int parse_file(const char* file_name, Output& output) {
    std::FILE* fp = std::fopen(file_name, "r");
    if (!fp) {
      return -1;
    }

    char readBuffer[8192];
    rapidjson::Document root;
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    if (root.ParseStream(is).HasParseError()) {
      return -1;
    }

    std::fclose(fp);
    return parse_json(root, output);
  }

  virtual int parse_json(const rapidjson::Value &root, Output& output) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_DATA_JSON_PARSER_H_ */
