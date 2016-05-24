#ifndef SRC_MAIN_UTILS_CONTAINER_UTILS_H_
#define SRC_MAIN_UTILS_CONTAINER_UTILS_H_

namespace redgiant {

// applies on std::map, std::unordered_map and map classes with same concepts
template <typename Map, typename Key>
auto map_find_or_null(const Map& m, const Key& key)
-> const typename Map::mapped_type * {
  auto iter = m.find(key);
  if (iter != m.end()) {
    return &(iter->second);
  }
  return nullptr;
}

template <typename Map, typename Key, typename Value>
auto map_find_or_default(const Map& m, const Key& key, Value default_value)
-> typename Map::mapped_type {
  auto iter = m.find(key);
  if (iter != m.end()) {
    return iter->second;
  }
  return default_value;
}

} /* namespace redgiant */

#endif /* SRC_MAIN_UTILS_CONTAINER_UTILS_H_ */
