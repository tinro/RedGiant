#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>

#include <libgen.h>
#include <signal.h>

#include "data/document_parser.h"
#include "data/feature_cache.h"
#include "data/query_request_parser.h"
#include "handler/document_handler.h"
#include "handler/query_handler.h"
#include "handler/test_handler.h"
#include "index/document_index_manager.h"
#include "index/document_index_view.h"
#include "index/document_update_pipeline.h"
#include "query/simple_query_executor.h"
#include "ranking/direct_model.h"
#include "ranking/feature_mapping_model.h"
#include "ranking/model_manager.h"
#include "ranking/ranking_model.h"
#include "service/server.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/filereadstream.h"
#include "utils/json_utils.h"
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
  const char* logger_file_name_str = json_try_get_string(config, kConfigKeyLoggerConfig);
  if (!logger_file_name_str) {
    fprintf(stderr, "Logger configuration not found! Using default configurations.");
    return init_logger(NULL);
  }

  // Copy the file name to a temporary buffer to call "dirname"
  size_t file_name_len = strlen(file_name);
  std::unique_ptr<char[]> file_name_buf(new char[file_name_len + 1]);
  strncpy(file_name_buf.get(), file_name, file_name_len);
  file_name_buf[file_name_len] = 0;
  std::string dir(dirname(file_name_buf.get()));
  if (!dir.empty() && dir.back() != '/') {
    dir += "/";
  }
  // get the file name relative to the configuration file.
  std::string logger_file_name = dir + std::string(logger_file_name_str);
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
  auto config_feature_spaces = json_try_get(config, kConfigKeyFeatureSpaces);
  if (!config_feature_spaces) {
    LOG_ERROR(logger, "features configuration does not exist!");
    return -1;
  }

  std::shared_ptr<FeatureCache> feature_cache = std::make_shared<FeatureCache>();
  if (feature_cache->initialize(*config_feature_spaces) < 0) {
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

  auto config_index = json_try_get_object(config, kConfigKeyIndex);
  if (config_index && json_try_get_int(*config_index, "initial_buckets", document_index_initial_buckets)) {
    LOG_DEBUG(logger, "document index initial buckets: %d", document_index_initial_buckets);
  } else {
    LOG_DEBUG(logger, "document index initial buckets not configured, use default: %d", document_index_initial_buckets);
  }
  if (config_index && json_try_get_int(*config_index, "max_size", document_index_max_size)) {
    LOG_DEBUG(logger, "document index max size: %d", document_index_max_size);
  } else {
    LOG_DEBUG(logger, "document index max size not configured, use default: %d", document_index_max_size);
  }
  if (config_index && json_try_get_int(*config_index, "maintain_interval", document_index_maintain_interval)) {
    LOG_DEBUG(logger, "document index maintain interval: %d", document_index_maintain_interval);
  } else {
    LOG_DEBUG(logger, "document index maintain interval not configured, use default: %d", document_index_maintain_interval);
  }

  DocumentIndexManager document_index(document_index_initial_buckets, document_index_max_size);
  document_index.start_maintain(document_index_maintain_interval, document_index_maintain_interval);
  ScopeGuard feed_document_index_guard([&document_index] {
    LOG_INFO(logger, "document index maintain thread stopping...");
    document_index.stop_maintain();
  });

  unsigned int document_update_thread_num = 4;
  unsigned int document_update_queue_size = 2048;
  unsigned int default_ttl = 86400;
  if (config_index && json_try_get_uint(*config_index, "update_thread_num", document_update_thread_num)) {
    LOG_DEBUG(logger, "feed document pipeline thread num: %u", document_update_thread_num);
  } else {
    LOG_DEBUG(logger, "feed document pipeline thread num not configured, use default: %u", document_update_thread_num);
  }
  if (config_index && json_try_get_uint(*config_index, "update_queue_size", document_update_queue_size)) {
    LOG_DEBUG(logger, "feed document pipeline queue size: %u", document_update_queue_size);
  } else {
    LOG_DEBUG(logger, "feed document pipeline queue size not configured, use default: %u", document_update_queue_size);
  }
  if (config_index && json_try_get_uint(*config_index, "default_ttl", default_ttl)) {
    LOG_DEBUG(logger, "document update default ttl: %u", default_ttl);
  } else {
    LOG_DEBUG(logger, "document update default ttl not configured, use default: %u", default_ttl);
  }

  DocumentUpdatePipeline document_update_pipeline(document_update_thread_num, document_update_queue_size, &document_index);
  document_update_pipeline.start();
  ScopeGuard feed_document_pipeline_guard([&document_update_pipeline] {
    LOG_INFO(logger, "feed document pipeline stopping...");
    document_update_pipeline.stop();
  });

  DocumentIndexView document_index_view(&document_index, &document_update_pipeline);

  /*
   * Initialization
   * Query and ranking models
   */
  ModelManagerFactory model_manager_factory;
  model_manager_factory.register_model_factory(std::make_shared<DirectModelFactory>());
  model_manager_factory.register_model_factory(std::make_shared<FeatureMappingModelFactory>(feature_cache));

  auto config_ranking = json_try_get(config, kConfigKeyRanking);
  if (!config_ranking) {
    LOG_ERROR(logger, "ranking model config does not exist!");
    return -1;
  }

  std::unique_ptr<RankingModel> model = model_manager_factory.create_model(*config_ranking);
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
  uint server_thread_num = 4;
  uint max_req_per_thread = 0;

  auto config_server = json_try_get_object(config, kConfigKeyServer);
  if (config_server && json_try_get_int(*config_server, "port", server_port)) {
    LOG_DEBUG(logger, "server port: %d", server_port);
  } else {
    LOG_DEBUG(logger, "server port not configured, use default: %d", server_port);
  }
  if (config_server && json_try_get_uint(*config_server, "thread_num", server_thread_num)) {
    LOG_DEBUG(logger, "server thread num: %u", server_thread_num);
  } else {
    LOG_DEBUG(logger, "server thread num not configured, use default: %u", server_thread_num);
  }
  if (config_server && json_try_get_uint(*config_server, "max_request_per_thread", max_req_per_thread)) {
    LOG_DEBUG(logger, "max requests per server thread: %u", max_req_per_thread);
  } else {
    LOG_DEBUG(logger, "max requests per server thread not configured, use default: %u", max_req_per_thread);
  }

  Server server(server_port, server_thread_num, max_req_per_thread);
  server.bind("/test", std::make_shared<TestHandlerFactory>());
  server.bind("/document", std::make_shared<FeedDocumentHandlerFactory>(
      std::make_shared<DocumentParserFactory>(feature_cache),
      &document_index_view, default_ttl));
  server.bind("/query", std::make_shared<QueryHandlerFactory>(
      std::make_shared<QueryRequestParserFactory>(feature_cache),
      std::make_shared<SimpleQueryExecutorFactory>(&document_index, model.get())));

  if (server.initialize() < 0) {
    LOG_ERROR(logger, "server initialization failed!");
    return -1;
  }

  if (server.start() < 0) {
    LOG_ERROR(logger, "failed to start server!");
    return -1;
  }

  ScopeGuard server_guard([&server] {
    LOG_INFO(logger, "server exiting...");
    server.stop();
  });

  LOG_INFO(logger, "service started successfully.");

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
    LOG_INFO(logger, "exit successfully.");
  } else {
    LOG_INFO(logger, "exit with failure.");
  }
  return ret;
}

} /* namespace redgiant */

int main(int argc, char** argv) {
  return redgiant::main(argc, argv);
}
