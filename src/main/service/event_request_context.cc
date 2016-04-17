#include "service/event_request_context.h"

#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <stdio.h>
#include <string.h>
#include "utils/logger.h"

DECLARE_LOGGER(logger, __FILE__);

namespace redgiant {

EventRequestContext::EventRequestContext(evhttp_request* req, std::shared_ptr<evhttp_uri> ev_uri)
: ev_req_(req), ev_uri_(std::move(ev_uri)) {
}

std::string EventRequestContext::get_uri() const {
  std::string uri = evhttp_request_get_uri(ev_req_);
  return uri;
}

std::string EventRequestContext::get_path() const {
  const char* path = evhttp_uri_get_path(ev_uri_.get());
  if (path == NULL) {
    return "/";
  } else {
    return path;
  }
}

void EventRequestContext::get_query_params(std::map<std::string, std::string>& out_params) const {
  evkeyvalq params;
  int ret = evhttp_parse_query_str(evhttp_uri_get_query(ev_uri_.get()), &params);
  if (ret != 0 ) {
    LOG_ERROR (logger, "decode query params error: %s", evhttp_uri_get_query(ev_uri_.get()));
    evhttp_clear_headers(&params);
    return;
  }

  evkeyval* entry;
  for (entry = params.tqh_first; entry != NULL; entry = entry->next.tqe_next) {
    //printf(" %s: %s\n", entry->key, entry->value);
    out_params[entry->key] = entry->value;
  }
  evhttp_clear_headers(&params);
}

std::string EventRequestContext::get_query_param(const char* key) const {
  evkeyvalq params;
  int ret = evhttp_parse_query_str(evhttp_uri_get_query(ev_uri_.get()), &params);
  if (ret != 0) {
    LOG_ERROR (logger, "decode query params error: %s", evhttp_uri_get_query(ev_uri_.get()));
    evhttp_clear_headers(&params);
    return "";
  }

  const char* ret_str = evhttp_find_header(&params, key);
  if (ret_str == NULL) {
    evhttp_clear_headers(&params);
    return "";
  }
  std::string ret_str2 = ret_str;
  evhttp_clear_headers(&params);
  return ret_str2;
}

int EventRequestContext::get_method() const {
  evhttp_cmd_type ev_cmd = evhttp_request_get_command(ev_req_);
  switch(ev_cmd) {
  case EVHTTP_REQ_GET:
    return METHOD_GET;
  case EVHTTP_REQ_POST:
    return METHOD_POST;
  case EVHTTP_REQ_HEAD:
    return METHOD_HEAD;
  case EVHTTP_REQ_PUT:
    return METHOD_PUT;
  case EVHTTP_REQ_DELETE:
    return METHOD_DELETE;
  case EVHTTP_REQ_OPTIONS:
    return METHOD_OPTIONS;
  case EVHTTP_REQ_TRACE:
    return METHOD_TRACE;
  case EVHTTP_REQ_CONNECT:
    return METHOD_CONNECT;
  case EVHTTP_REQ_PATCH:
    return METHOD_PATCH;
  default:
    return METHOD_UNKNOWN;
  }
  return METHOD_UNKNOWN;
}

int EventRequestContext::get_post_length() const {
  evbuffer* buf = evhttp_request_get_input_buffer(ev_req_);
  return evbuffer_get_length(buf);
}

int EventRequestContext::get_post_content(char* out_buf, int max_len) const {
  evbuffer* buf = evhttp_request_get_input_buffer(ev_req_);
  int post_len = evbuffer_get_length(buf);
  int ret_len = post_len < max_len? post_len: max_len;
  return evbuffer_remove(buf, out_buf, ret_len);
}

std::string EventRequestContext::get_header(const char* key) const {
  evkeyvalq *headers = ev_req_->input_headers;
  const char* ret = evhttp_find_header(headers, key);
  if (ret == NULL)
    return "";
  return ret;
}

} // namespace redgiant
