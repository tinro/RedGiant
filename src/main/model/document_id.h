#ifndef SRC_MAIN_MODEL_DOCUMENT_ID_H_
#define SRC_MAIN_MODEL_DOCUMENT_ID_H_

#include <cstdint>
#include <string>

namespace redgiant {
class DocumentId {
public:
  DocumentId() // invalid id
  : low_(0), high_(0) {
  }

  DocumentId(uint64_t low, uint64_t high = 0)
  : low_(low), high_(high) {
  }

  DocumentId(const std::string& uuid);

  ~DocumentId() = default;

  bool operator== (const DocumentId& rhs) const {
    return low_ == rhs.low_ && high_ == rhs.high_;
  }

  bool operator!= (const DocumentId& rhs) const {
    return low_ != rhs.low_ || high_ != rhs.high_;
  }

  bool operator< (const DocumentId& rhs) const {
    return high_ < rhs.high_ || (high_ == rhs.high_ && low_ < rhs.low_);
  }

  bool operator<= (const DocumentId& rhs) const {
    return high_ < rhs.high_ || (high_ == rhs.high_ && low_ <= rhs.low_);
  }

  bool operator> (const DocumentId& rhs) const {
    return high_ > rhs.high_ || (high_ == rhs.high_ && low_ > rhs.low_);
  }

  bool operator>= (const DocumentId& rhs) const {
    return high_ > rhs.high_ || (high_ == rhs.high_ && low_ >= rhs.low_);
  }

  operator bool () const {
    return low_ || high_;
  }

  DocumentId& operator++ () {
    ++low_;
    if (low_ == 0) {
      ++high_;
    }
    return *this;
  }

  DocumentId& operator-- () {
    if (low_ == 0) {
      --low_; --high_;
    } else {
      --low_;
    }
    return *this;
  }

  std::string to_string() const;

  struct Hash {
    size_t operator()(const DocumentId& id) const noexcept {
      return id.low_ ^ id.high_;
    }
  };

private:
  uint64_t low_;
  uint64_t high_;

  union GuidUnion {
    struct {
      uint64_t low;
      uint64_t high;
    } __attribute__((__packed__)) st;
    struct {
      uint32_t data1[1];
      uint16_t data2[1];
      uint16_t data3[1];
      uint8_t data4[8];
    } __attribute__((__packed__)) data;
  } __attribute__((__packed__));
};
} /* namespace redgiant */

#endif /* SRC_MAIN_INDEX_DOCUMENT_ID_H_ */
