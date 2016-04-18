#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>
#include <signal.h>

#include "handler/test_handler.h"
#include "service/server.h"
#include "utils/logger.h"
#include "utils/scope_guard.h"

using std::string;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

static volatile int g_exit_signal = 0;

static void new_handler_abort() {
  std::abort();
}

void ignore_signal(int signal) {
  (void) signal;
}

void exit_on_signal(int signal) {
  (void) signal;
  g_exit_signal = signal;
}

int server_main() {
  // using SIG_IGN caused seg fault, don't know why
  // the simple empty function works
  signal(SIGPIPE, ignore_signal);
  signal(SIGHUP, ignore_signal);
  signal(SIGTERM, exit_on_signal);
  signal(SIGINT, exit_on_signal);

  LOG_INFO(logger, "server initializing ...");
  Server test_server(19980, 2, 1024);
  test_server.bind("/test", std::make_shared<TestHandlerFactory>());
  if (test_server.initialize() < 0) {
    LOG_ERROR(logger, "test server initialization failed!");
    return -1;
  }

  ScopeGuard test_server_guard([&test_server] {
    LOG_INFO(logger, "test server exiting...");
    test_server.stop();
  });

  if (test_server.start() < 0) {
    LOG_ERROR(logger, "failed to start test server!");
    return -1;
  }
  LOG_INFO(logger, "test server stared.");

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGINT);

  sigset_t orig_mask;
  sigprocmask(SIG_BLOCK, &mask, &orig_mask);
  g_exit_signal = 0;
  while (!g_exit_signal) {
    sigsuspend(&orig_mask);
  }
  
  LOG_INFO(logger, "received signal %d. exiting.", g_exit_signal);
  return 0;
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  std::set_new_handler(new_handler_abort);

  if (argc == 1) {
    fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
    return -1;
  }

  if (argc != 2) {
    fprintf(stderr, "Argument error.\n");
    return -1;
  }

  init_logger(argv[1]);

  int ret = -1;
  try {
    ret = server_main();
  } catch (...) {
    // don't make this happen
    ret = -1;
    LOG_ERROR(logger, "unkown error happened");
  }

  if (ret == 0) {
    LOG_INFO(logger, "server exit ALL OK.");
  } else {
    LOG_INFO(logger, "server exit OK with failure.");
  }
  return ret;
}

} /* namespace redgiant */

int main(int argc, char** argv) {
  return redgiant::main(argc, argv);
}
