#ifndef REDGIANT_UTILS_LOGGER_H_
#define REDGIANT_UTILS_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <log4cxx/logger.h>

namespace redgiant {

inline char* log_format_internal__(char* buf, size_t n, const char* fmt, ...) __attribute__((format(printf,3,4)));

// format the log string using C-style printf function, return the string
inline char* log_format_internal__(char* buf, size_t n, const char* fmt, ...) {
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, n-1, fmt, args);
  va_end (args);
  buf[n-1] = 0;
  return buf;
}

#define DECLARE_LOGGER(logger, name) static log4cxx::Logger* logger = log4cxx::Logger::getLogger(name)

// the log_format_internal__ function is lazy evaluated in the LOG4CXX_LOG macro
#define LOG_INTERNAL__(logger, size, level, ...) do { \
  char msg_buf[size]; \
  LOG4CXX_LOG(logger, level, log_format_internal__(msg_buf, size, __VA_ARGS__)); \
} while (false)

// normally, the INFO log will not be too long
#define LOG_INTERNAL_MAX_SIZE_INFO__  1024
// DEBUG log is used for printing debug info for queries marked as debug
#define LOG_INTERNAL_MAX_SIZE_DEBUG__ 4096
// TRACE log may be very long
#define LOG_INTERNAL_MAX_SIZE_TRACE__ 65536

#define LOG_TRACE(logger, ...)  LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_TRACE__, log4cxx::Level::getTrace(), __VA_ARGS__)
#define LOG_DEBUG(logger, ...)  LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_DEBUG__, log4cxx::Level::getDebug(), __VA_ARGS__)
#define LOG_INFO(logger, ...)   LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_INFO__,  log4cxx::Level::getInfo(),  __VA_ARGS__)
#define LOG_WARN(logger, ...)   LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_INFO__,  log4cxx::Level::getWarn(),  __VA_ARGS__)
#define LOG_ERROR(logger, ...)  LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_INFO__,  log4cxx::Level::getError(), __VA_ARGS__)
#define LOG_FATAL(logger, ...)  LOG_INTERNAL__(logger, LOG_INTERNAL_MAX_SIZE_INFO__,  log4cxx::Level::getFatal(), __VA_ARGS__)

} // namespace redgiant

#endif /* REDGIANT_UTILS_LOGGER_H_ */
