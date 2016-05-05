#include "model/document_id.h"

#include <cstdio>

namespace redgiant {

DocumentId::DocumentId(const std::string& uuid)
: low_(0), high_(0) {
  GuidUnion u;
  int ret = std::sscanf(uuid.c_str(), "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
      u.data.data1, u.data.data2, u.data.data3,
      u.data.data4, u.data.data4+1, u.data.data4+2, u.data.data4+3,
      u.data.data4+4, u.data.data4+5, u.data.data4+6, u.data.data4+7);
  if (ret == 11) {
    low_ = u.st.low;
    high_ = u.st.high;
  }
  // else keep zero
}

std::string DocumentId::to_string() const {
  GuidUnion u;
  u.st.low = low_;
  u.st.high = high_;
  char buf[37];
  std::sprintf(buf, "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
      u.data.data1[0], u.data.data2[0], u.data.data3[0],
      u.data.data4[0], u.data.data4[1], u.data.data4[2], u.data.data4[3],
      u.data.data4[4], u.data.data4[5], u.data.data4[6], u.data.data4[7]);
  return buf;
}

} /* namespace redgiant */
