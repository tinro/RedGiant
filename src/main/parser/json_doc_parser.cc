#include "parser/json_doc_parser.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

#include "utils/logger.h"

using namespace std;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

std::unique_ptr<DocFeatures> JsonDocParser::parse_node(const rapidjson::Value& root) {
  if (!root.IsObject()) {
    LOG_ERROR(logger, "document does not exist.");
    return nullptr;
  }

  std::string uuid;
  if (root["uuid"].IsString())
    uuid = root["uuid"].GetString();
  else {
    LOG_ERROR(logger, "document uuid missing");
    return nullptr;
  }

  std::unique_ptr<DocFeatures> doc(new DocFeatures(std::move(uuid)));
  if (root["features"].IsArray()) {
    for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
      if (it->name.IsString() && it->value.IsArray()) {
      }
    }
  }

  return doc;
}

} /* namespace redgiant */
