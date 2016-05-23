#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>
// Linux headers
#include <libgen.h>
#include <signal.h>

#include "handler/feed_document_handler.h"
#include "handler/query_handler.h"
#include "handler/test_handler.h"
#include "index/document_index_manager.h"
#include "parser/document_parser.h"
#include "parser/feature_cache_parser.h"
#include "parser/query_request_parser.h"
#include "pipeline/feed_document_pipeline.h"
#include "query/simple_query_executor.h"
#include "ranking/default_model.h"
#include "ranking/feature_mapping_model.h"
#include "ranking/model_manager.h"
#include "ranking/ranking_model.h"
#include "service/server.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/filereadstream.h"
#include "utils/logger.h"
#include "utils/logger-inl.h"
#include "utils/scope_guard.h"

using std::string;

namespace redgiant {

DECLARE_LOGGER(logger, __FILE__);

// constants used by configuration files
static const unsigned int kConfigParseFlags =
    rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag;
static const char kConfigKeyLoggerConfig    [] = "logger_config";
static const char kConfigKeyFeatureSpaces   [] = "feature_spaces";
static const char kConfigKeyIndex           [] = "index";
static const char kConfigKeyRanking         [] = "ranking";
static const char kConfigKeyPipeline        [] = "pipeline";
static const char kConfigKeyServer          [] = "server";

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

static int read_config_file(const char* file_name, rapidjson::Document& config) {
  std::FILE* fp = std::fopen(file_name, "r");
  if (!fp) {
    fprintf(stderr, "Cannot open config file %s.\n", file_name);
    return -1;
  }

  char readBuffer[8192];
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  if (config.ParseStream<kConfigParseFlags>(is).HasParseError()) {
    fprintf(stderr, "Config file parse error %d at offset %zu.\n",
        (int)config.GetParseError(), config.GetErrorOffset());
    return -1;
  }

  std::fclose(fp);
  return 0;
}

static int init_log_config(const char* file_name, const rapidjson::Value& config) {
  if (!config.HasMember(kConfigKeyLoggerConfig) || !config[kConfigKeyLoggerConfig].IsString()) {
    fprintf(stderr, "No log file configured!");
    return init_logger(NULL);
  }

  // Copy the file name to a temporary buffer to call "dirname"
  size_t file_name_len = strlen(file_name);
  std::unique_ptr<char[]> file_name_buf(new char[file_name_len + 1]);
  strncpy(file_name_buf.get(), file_name, file_name_len);
  file_name_buf[file_name_len] = 0;
  std::string dir = dirname(file_name_buf.get());
  if (dir.back() != '/') {
    dir += "/";
  }

  // get the file name relative to the configuration file.
  std::string logger_file_name = dir + config[kConfigKeyLoggerConfig].GetString();
  return init_logger(logger_file_name.c_str());
}

static int server_main(rapidjson::Value& config) {
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
  if (!config.HasMember(kConfigKeyFeatureSpaces)) {
    LOG_ERROR(logger, "features configuration does not exist!");
    return -1;
  }

  std::shared_ptr<FeatureCache> feature_cache = std::make_shared<FeatureCache>();
  FeatureCacheParser feature_cache_parser;
  if (feature_cache_parser.parse_json(config[kConfigKeyFeatureSpaces], *feature_cache) < 0) {
    LOG_ERROR(logger, "feature cache parsing failed!");
    return -1;
  }

  /*
   * Initialization:
   * Index initialization
   */
  int document_index_initial_buckets    = 100000;
  int document_index_max_size           = 5000000;
  int document_index_maintain_interval  = 300;

  if (config.HasMember(kConfigKeyIndex)) {
    auto& index_config = config[kConfigKeyIndex];
    if (index_config.HasMember("initial_buckets") && index_config["initial_buckets"].IsInt()) {
      document_index_initial_buckets = index_config["initial_buckets"].GetInt();
    }
    if (index_config.HasMember("max_size") && index_config["max_size"].IsInt()) {
      document_index_max_size = index_config["max_size"].GetInt();
    }
    if (index_config.HasMember("maintain_interval") && index_config["maintain_interval"].IsInt()) {
      document_index_maintain_interval = index_config["max_size"].GetInt();
    }
  }

  DocumentIndexManager document_index(document_index_initial_buckets, document_index_max_size);
  document_index.start_maintain(document_index_maintain_interval, document_index_maintain_interval);
  ScopeGuard feed_document_index_guard([&document_index] {
    LOG_INFO(logger, "document index maintain thread stopping...");
    document_index.stop_maintain();
  });

  size_t feed_document_thread_num = 4;
  size_t feed_document_queue_size = 2048;
  if (config.HasMember(kConfigKeyPipeline)) {
    auto& pipeline_config = config[kConfigKeyPipeline];
    if (pipeline_config.HasMember("thread_num") && pipeline_config["thread_num"].IsInt()) {
      feed_document_thread_num = pipeline_config["thread_num"].GetInt();
    }
    if (pipeline_config.HasMember("queue_size") && pipeline_config["queue_size"].IsInt()) {
      feed_document_queue_size = pipeline_config["queue_size"].GetInt();
    }
  }

  FeedDocumentPipeline feed_document(feed_document_thread_num, feed_document_queue_size, &document_index);
  feed_document.start();
  ScopeGuard feed_document_pipeline_guard([&feed_document] {
    LOG_INFO(logger, "feed document pipeline stopping...");
    feed_document.stop();
  });

  /*
   * Initialization
   * Query and ranking models
   */
  ModelManagerFactory model_manager_factory;
  model_manager_factory.register_model_factory(std::make_shared<DefaultModelFactory>());
  model_manager_factory.register_model_factory(std::make_shared<FeatureMappingModelFactory>(feature_cache));

  if (!config.HasMember(kConfigKeyRanking)) {
    LOG_ERROR(logger, "ranking model config does not exist!");
    return -1;
  }

  std::unique_ptr<RankingModel> model = model_manager_factory.create_model(config[kConfigKeyRanking]);
  if (!model) {
    LOG_ERROR(logger, "ranking model initialization failed!");
    return -1;
  }

  /*
   * Initialization:
   * Server initialization
   */
  LOG_INFO(logger, "server initializing ...");
  int server_port = 19980;
  size_t server_thread_num = 4;
  size_t max_req_per_thread = 0;
  if (config.HasMember(kConfigKeyServer)) {
    auto& server_config = config[kConfigKeyServer];
    if (server_config.HasMember("port") && server_config["port"].IsInt()) {
      server_port = server_config["port"].GetInt();
    }
    if (server_config.HasMember("thread_num") && server_config["thread_num"].IsInt()) {
      server_thread_num = server_config["thread_num"].GetInt();
    }
    if (server_config.HasMember("max_request_per_thread") && server_config["max_request_per_thread"].IsInt()) {
      max_req_per_thread = server_config["max_request_per_thread"].GetInt();
    }
  }


  Server test_server(server_port, server_thread_num, max_req_per_thread);
  test_server.bind("/test", std::make_shared<TestHandlerFactory>());
  test_server.bind("/document", std::make_shared<FeedDocumentHandlerFactory>(
      std::make_shared<DocumentParserFactory>(feature_cache),
      &feed_document, 0));
  test_server.bind("/query", std::make_shared<QueryHandlerFactory>(
      std::make_shared<QueryRequestParserFactory>(feature_cache),
      std::make_shared<SimpleQueryExecutorFactory>(&document_index, model.get())));

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

  if (argc != 2) {
    fprintf(stderr, "Usage: %s config_file\n", argv[0]);
    return -1;
  }

  rapidjson::Document config;
  if (read_config_file(argv[1], config) < 0) {
    fprintf(stderr, "Failed to open config file %s.\n", argv[1]);
    return -1;
  }

  if (init_log_config(argv[1], config) < 0) {
    fprintf(stderr, "Failed to initialize log config.\n");
    return -1;
  }

  int ret = -1;
  try {
    ret = server_main(config);
  } catch (...) {
    // don't make this happen
    ret = -1;
    LOG_ERROR(logger, "unkown error happened");
  }

  if (ret >= 0) {
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
