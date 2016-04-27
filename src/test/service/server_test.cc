#include <memory>
#include <utility>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "service/server.h"
#include "service/server_instance.h"
#include "utils/logger.h"
#include "utils/scope_guard.h"
#include "mock_handler.h"
#include "test_client.h"

namespace redgiant {
class ServerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ServerTest);
  CPPUNIT_TEST(test_server);
  CPPUNIT_TEST_SUITE_END();

public:
  ServerTest() = default;
  virtual ~ServerTest() = default;

protected:
  void test_server() {
    Server server(port_, 2, 1024);
    server.bind("/test", std::make_shared<MockHandlerFactory>(message_));

    int ret = server.initialize();
    CPPUNIT_ASSERT_EQUAL(ret, 0);

    // fd available
    CPPUNIT_ASSERT(server.fd_ > 0);
    CPPUNIT_ASSERT(server.notify_fd_[0] > 0);
    CPPUNIT_ASSERT(server.notify_fd_[1] > 0);
    CPPUNIT_ASSERT_EQUAL(2, (int)server.instances_.size());
    // check the instance
    CPPUNIT_ASSERT(server.instances_[0]->fd_ == server.fd_);
    CPPUNIT_ASSERT(server.instances_[0]->notify_fd_ == server.notify_fd_[0]);
    CPPUNIT_ASSERT_EQUAL(1024, (int)server.instances_[0]->max_req_per_thread_);
    CPPUNIT_ASSERT(server.instances_[0]->route_map_["/test"]);
    // another instance: the same
    CPPUNIT_ASSERT(server.instances_[1]->fd_ == server.fd_);
    CPPUNIT_ASSERT(server.instances_[1]->notify_fd_ == server.notify_fd_[0]);
    CPPUNIT_ASSERT_EQUAL(1024, (int)server.instances_[1]->max_req_per_thread_);
    CPPUNIT_ASSERT(server.instances_[1]->route_map_["/test"]);

    // another server on the same port
    Server server2(port_, 1, 1);

    {
      server.start(); // started without exceptions
      ScopeGuard server_guard([&server] { server.stop(); });
      CPPUNIT_ASSERT_EQUAL(2, (int)server.instance_threads_.size());

      // start http client to the service
      TestClient client;
      client.request("127.0.0.1", port_, EVHTTP_REQ_GET, "/test");
      CPPUNIT_ASSERT_EQUAL(message_, client.response());

      // start another server on the same port shall fail
      ret = server2.initialize();
      CPPUNIT_ASSERT_EQUAL(ret, -1);
      CPPUNIT_ASSERT(server2.fd_ < 0);
      CPPUNIT_ASSERT(server2.notify_fd_[0] < 0);
      CPPUNIT_ASSERT(server2.notify_fd_[1] < 0);
    } // will call stop

    // fd closed
    CPPUNIT_ASSERT(server2.fd_ < 0);
    CPPUNIT_ASSERT(server2.notify_fd_[0] < 0);
    CPPUNIT_ASSERT(server2.notify_fd_[1] < 0);
    // cleaned
    CPPUNIT_ASSERT_EQUAL(0, (int)server.instances_.size());
    CPPUNIT_ASSERT_EQUAL(0, (int)server.instance_threads_.size());

    ret = server2.initialize(); // port not used
    CPPUNIT_ASSERT_EQUAL(ret, 0);
  }

protected:
  std::string message_ = "done!";
  int port_ = 49988;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServerTest);

} /* namespace redgiant */
