#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>
#include <signal.h>

#include "handler/feed_document_handler.h"
#include "handler/test_handler.h"
#include "index/document_index_manager.h"
#include "parser/document_parser.h"
#include "parser/feature_cache_parser.h"
#include "pipeline/feed_document_pipeline.h"
#include "service/server.h"
#include "utils/logger.h"
#include "utils/logger-inl.h"
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
  /*
   * Initialize signal handler
   */
  signal(SIGPIPE, ignore_signal);
  signal(SIGHUP, ignore_signal);
  signal(SIGTERM, exit_on_signal);
  signal(SIGINT, exit_on_signal);

  /*
   * Initialization:
   * Features initialization
   */
  FeatureCache feature_cache;
  FeatureCacheParser feature_cache_parser;
  if (feature_cache_parser.parse_file("./conf/feature_space.json", feature_cache) < 0) {
    LOG_ERROR(logger, "feature cache parsing failed!");
    return -1;
  }

  /*
   * Initialization:
   * Index initialization
   */
  DocumentIndexManager document_index(500000, 500000);
  document_index.start_maintain(10, 60);
  ScopeGuard feed_document_index_guard([&document_index] {
    LOG_INFO(logger, "document index maintain thread stopping...");
    document_index.stop_maintain();
  });

  FeedDocumentPipeline feed_document(4, 2048, &document_index);
  feed_document.start();
  ScopeGuard feed_document_pipeline_guard([&feed_document] {
    LOG_INFO(logger, "feed document pipeline stopping...");
    feed_document.stop();
  });

  DocumentParser document_parser(&feature_cache);
  /*
   * Initialize:
   * Server initialization
   */
  LOG_INFO(logger, "server initializing ...");
  Server test_server(19980, 2, 1024);
  test_server.bind("/test", std::make_shared<TestHandlerFactory>());
  test_server.bind("/document", std::make_shared<FeedDocumentHandlerFactory>(&feed_document, &document_parser, 0));

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

  /*
   * Main loop: wait for exit signal.
   */
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

  /*
   * Exit: everything exits elegantly
   */
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
