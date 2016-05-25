#ifndef SRC_MAIN_UTILS_JSON_UTILS_H_
#define SRC_MAIN_UTILS_JSON_UTILS_H_

#include "third_party/rapidjson/document.h"

namespace redgiant {

inline const rapidjson::Value* json_try_get(const rapidjson::Value& json, const char* key) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    return &(iter->value);
  }
  return nullptr;
}

inline const rapidjson::Value* json_try_get_array(const rapidjson::Value& json, const char* key) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsArray()) {
      return &value;
    }
  }
  return nullptr;
}

inline const rapidjson::Value* json_try_get_object(const rapidjson::Value& json, const char* key) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsObject()) {
      return &value;
    }
  }
  return nullptr;
}

inline const char* json_try_get_string(const rapidjson::Value& json, const char* key) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsString()) {
      return value.GetString();
    }
  }
  return nullptr;
}

inline bool json_try_get_string(const rapidjson::Value& json, const char* key, std::string& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsString()) {
      ret = value.GetString();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_int(const rapidjson::Value& json, const char* key, int& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsInt()) {
      ret = value.GetInt();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_uint(const rapidjson::Value& json, const char* key, unsigned int & ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsUint()) {
      ret = value.GetUint();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_int64(const rapidjson::Value& json, const char* key, int64_t& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsInt64()) {
      ret = value.GetInt64();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_int64(const rapidjson::Value& json, const char* key, uint64_t& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsUint64()) {
      ret = value.GetUint64();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_double(const rapidjson::Value& json, const char* key, double& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsDouble()) {
      ret = value.GetDouble();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_float(const rapidjson::Value& json, const char* key, float& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsFloat()) {
      ret = value.GetFloat();
      return true;
    }
  }
  return false;
}

inline bool json_try_get_bool(const rapidjson::Value& json, const char* key, bool& ret) {
  auto iter = json.FindMember(key);
  if (iter != json.MemberEnd()) {
    const auto& value = iter->value;
    if (value.IsBool()) {
      ret = value.GetBool();
      return true;
    }
  }
  return false;
}

} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_JSON_UTILS_H_ */
