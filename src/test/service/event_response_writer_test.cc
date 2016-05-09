#include <memory>
#include <utility>
#include <vector>
#include <event2/buffer.h>
#include <event2/event.h>
#include <evhttp.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "service/event_response_writer.h"

using namespace std;

namespace redgiant {
class EventResponseWriterTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EventResponseWriterTest);
  CPPUNIT_TEST(test_response);
  CPPUNIT_TEST(test_error);
  CPPUNIT_TEST_SUITE_END();

public:
  EventResponseWriterTest() = default;
  virtual ~EventResponseWriterTest() = default;

protected:
  void test_response() {
    evhttp_request* req = evhttp_request_new(NULL, NULL);
    unique_ptr<ResponseWriter> writer(new EventResponseWriter(req));
    writer->add_header("Connection", "close");
    string message1 = "Hello,";
    string message2 = "World!";
    writer->add_body(message1);
    writer->add_body(message2);
    writer->send(200, NULL);
  }

  void test_error() {
    evhttp_request* req = evhttp_request_new(NULL, NULL);
    unique_ptr<ResponseWriter> writer(new EventResponseWriter(req));
    writer->add_header("Connection", "close");
    writer->send_empty(400, NULL);
  }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(EventResponseWriterTest);

} /* namespace redgiant */
