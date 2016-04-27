#ifndef SRC_MAIN_REDGIANT_DATA_PARSER_H_
#define SRC_MAIN_REDGIANT_DATA_PARSER_H_

#include <utility>

namespace redgiant {
template <typename Output>
class Parser {
public:
  Parser() = default;
  virtual ~Parser() = default;

  virtual std::unique_ptr<Output> parse(const char* str, size_t len) = 0;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_REDGIANT_DATA_PARSER_H_ */
