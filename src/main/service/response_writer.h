#ifndef _REDGIANT_SERVICE_RESPONSE_WRITER_H_
#define _REDGIANT_SERVICE_RESPONSE_WRITER_H_

#include <cstring>
#include <string>

namespace redgiant {
class ResponseWriter {
public:
  ResponseWriter() = default;
  virtual ~ResponseWriter() = default;

  virtual void add_header(const char* key, const char* value) = 0;
  virtual void add_header(const std::string& key, const std::string& value) {
    add_header(key.c_str(), value.c_str());
  }

  virtual void add_body(const void* body, size_t size) = 0;
  virtual void add_body(const char* body) {
    add_body(body, std::strlen(body));
  }
  virtual void add_body(const std::string& body) {
    add_body(body.c_str(), body.size());
  }

  virtual void send(int status_code, const char* status_msg) = 0;
  virtual void send_empty(int status_code, const char* status_msg) = 0;
};
} // namespace redgiant

#endif // _REDGIANT_SERVICE_RESPONSE_WRITER_H_
