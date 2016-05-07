#ifndef SRC_MAIN_PARSER_PARSER_H_
#define SRC_MAIN_PARSER_PARSER_H_

namespace redgiant {
template <typename Output>
class Parser {
public:
  Parser() = default;
  virtual ~Parser() = default;

  virtual int parse_file(const char* file_name, Output& output) = 0;

  virtual int parse(const char* str, size_t len, Output& output) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_PARSER_PARSER_H_ */
