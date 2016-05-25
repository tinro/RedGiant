#ifndef SRC_MAIN_UTILS_STRING_UTILS_H_
#define SRC_MAIN_UTILS_STRING_UTILS_H_

#include <string>
#include <vector>

namespace redgiant {

std::vector<std::string> string_split(const std::string& str, char delim)  {
  std::vector<std::string> output;
  size_t start = std::string::npos;
  for (size_t i = 0; i <= str.length(); ++i) {
    if (start == std::string::npos) {
      start = i;
    }
    if (i == str.length() || str[i] == delim) {
      // skip if the start point itself is a delimiter
      output.emplace_back(str.c_str() + start, str.c_str() + i);
      start = std::string::npos;
    }
  }
  return output;
}

std::string string_strip(const std::string& str) {
  const char* trims = " \t\n\r";
  size_t begin = str.find_first_not_of(trims);
  if (begin != std::string::npos) {
    size_t end = str.find_last_not_of(trims);
    // end shall also not npos, and not less than begin
    return std::string(&str[begin], &str[end+1]);
  }
  return "";
}

} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_STRING_UTILS_H_ */
