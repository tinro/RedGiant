#ifndef SRC_MAIN_CORE_SNAPSHOT_SNAPSHOT_H_
#define SRC_MAIN_CORE_SNAPSHOT_SNAPSHOT_H_

#include <fstream>
#include <type_traits>

namespace redgiant {
class SnapshotDumper {
public:
  SnapshotDumper(const std::string& file_name)
  : ofs_(file_name) {
    ofs_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  }

  // disable copy
  SnapshotDumper(const SnapshotDumper&) = delete;
  SnapshotDumper& operator= (const SnapshotDumper&) = delete;

  // enable move
  SnapshotDumper(SnapshotDumper&&) = default;
  SnapshotDumper& operator= (SnapshotDumper&&) = default;

  ~SnapshotDumper() = default;

  // write an object with type T to stream
  // note: T must be trivially copyable to be adaptable to write to/read from stream
  // note: T may be reference type to be adaptable to rvalues
  // note: return size_t if it is enabled, otherwise won't compile
  // note: typically, the constraint for classes safe to serialization/deserialiation
  // should be is_trivially_copyable, but gcc does not support this, and is_trivial does
  // not apply to our doc id classes, so I removed the constrains here.
  template <typename T>
//  typename std::enable_if<
//    std::is_trivially_copyable<typename std::remove_reference<T>::type>::value,
//    size_t>::type
  size_t
  dump(T&& t) {
    ofs_.write(reinterpret_cast<const char*>(&t), sizeof(t));
    return sizeof(t);
  }

private:
  std::ofstream ofs_;
};

class SnapshotLoader {
public:
  SnapshotLoader(const std::string& file_name)
  : ifs_(file_name) {
    ifs_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  }

  // disable copy
  SnapshotLoader(const SnapshotLoader&) = delete;
  SnapshotLoader& operator= (const SnapshotLoader&) = delete;

  // enable move
  SnapshotLoader(SnapshotLoader&&) = default;
  SnapshotLoader& operator= (SnapshotLoader&&) = default;

  ~SnapshotLoader() = default;

  // write an object with type T to stream
  // note: T must be trivially copyable to be adaptable to write to/read from stream
  // note: T may be reference type to be adaptable to rvalues
  // note: return size_t if it is enabled, otherwise won't compile
  // note: typically, the constraint for classes safe to serialization/deserialiation
  // should be is_trivially_copyable, but gcc does not support this, and is_trivial does
  // not apply to our doc id classes, so I removed the constrains here.
  template <typename T>
  //  typename std::enable_if<
  //    std::is_trivially_copyable<typename std::remove_reference<T>::type>::value,
  //    size_t>::type
  size_t
  load(T&& t) {
    ifs_.read(reinterpret_cast<char*>(&t), sizeof(t));
    return sizeof(t);
  }

private:
  std::ifstream ifs_;
};
} /* namespace redgiant */

#endif /* SRC_MAIN_CORE_SNAPSHOT_SNAPSHOT_H_ */
