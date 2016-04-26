#include <memory>
#include <thread>
#include <utility>
#include <vector>
#include <event2/buffer.h>
#include <event2/event.h>
#include <evhttp.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "service/event_request_context.h"

using namespace std;

namespace redgiant {
class EventRequestContextTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EventRequestContextTest);
  CPPUNIT_TEST(test_request);
  CPPUNIT_TEST(test_post);
  CPPUNIT_TEST_SUITE_END();

public:
  EventRequestContextTest() = default;
  virtual ~EventRequestContextTest() = default;

protected:
  void test_request() {
    shared_ptr<event_base> base(event_base_new(), event_base_free);
    shared_ptr<evhttp_connection> conn(
        evhttp_connection_base_new(base.get(), NULL, "127.0.0.1", port_),
        evhttp_connection_free);

    evhttp_request* req = evhttp_request_new(NULL, NULL);
    evhttp_add_header(req->output_headers, "Host", "localhost");
    evhttp_make_request(conn.get(), req, EVHTTP_REQ_GET, "/test/query?key=abc&from=test");

    std::shared_ptr<evhttp_uri> uri(
        evhttp_uri_parse(evhttp_request_get_uri(req)),
        evhttp_uri_free);
    // create target
    EventRequestContext req_ctx(req, uri);

    CPPUNIT_ASSERT_EQUAL(string("/test/query?key=abc&from=test"), req_ctx.get_uri());
    CPPUNIT_ASSERT_EQUAL(string("/test/query"), req_ctx.get_path());
    CPPUNIT_ASSERT_EQUAL(string("abc"), req_ctx.get_query_param("key"));
    CPPUNIT_ASSERT_EQUAL(string("test"), req_ctx.get_query_param("from"));
    // empty result
    CPPUNIT_ASSERT_EQUAL(string(""), req_ctx.get_query_param("to"));
    CPPUNIT_ASSERT_EQUAL((int)RequestContext::METHOD_GET, req_ctx.get_method());

    map<string, string> params;
    req_ctx.get_query_params(params);
    CPPUNIT_ASSERT_EQUAL(2, (int)params.size());
    CPPUNIT_ASSERT_EQUAL(string("abc"), params["key"]);
    CPPUNIT_ASSERT_EQUAL(string("test"), params["from"]);
  }

  void test_post() {
    shared_ptr<event_base> base(event_base_new(), event_base_free);
    shared_ptr<evhttp_connection> conn(
        evhttp_connection_base_new(base.get(), NULL, "127.0.0.1", port_),
        evhttp_connection_free);

    evhttp_request* req = evhttp_request_new(NULL, NULL);
    evbuffer* buf = evhttp_request_get_input_buffer(req);
    string content = "test post content";
    evbuffer_add(buf, content.c_str(), content.size());

    evhttp_add_header(req->output_headers, "Host", "localhost");
    evhttp_make_request(conn.get(), req, EVHTTP_REQ_POST, "/query");

    std::shared_ptr<evhttp_uri> uri(
        evhttp_uri_parse(evhttp_request_get_uri(req)),
        evhttp_uri_free);
    // create target
    EventRequestContext req_ctx(req, uri);

    CPPUNIT_ASSERT_EQUAL(string("/query"), req_ctx.get_path());
    CPPUNIT_ASSERT_EQUAL((int)RequestContext::METHOD_POST, req_ctx.get_method());
    CPPUNIT_ASSERT_EQUAL((int)content.size(), req_ctx.get_post_length());

    char cbuf[1024];
    int ret = req_ctx.get_post_content(cbuf, 1023);
    CPPUNIT_ASSERT_EQUAL((int)content.size(), ret);
    cbuf[ret] = 0;
    CPPUNIT_ASSERT_EQUAL(content, string(cbuf));
  }

protected:
  int port_ = 49988;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EventRequestContextTest);

} /* namespace redgiant */
