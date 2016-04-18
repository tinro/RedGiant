#include "utils/logger.h"

#include <stdio.h>
#include <string.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>

using namespace log4cxx;

namespace redgiant {

int init_logger(const char* conf_path) {
  try {
    if (conf_path == NULL || strlen(conf_path) == 0) {
      printf ("Warning: Using default logger\n");
      log4cxx::BasicConfigurator::configure();
    } else {
      printf ("Using logger defined in %s\n", conf_path);
      if (strstr(conf_path, ".xml") != NULL) {
        printf ("Using log4cxx XML configurator\n");
        log4cxx::xml::DOMConfigurator::configure(conf_path);
      } else {
        printf ("Using log4cxx Property configurator\n");
        log4cxx::PropertyConfigurator::configure(conf_path);
      }
    }
  } catch (log4cxx::helpers::Exception& e) {
    printf ("Failed init logger: %s\n", e.what());
    return -1;
  }
  return 0;
}

} // namespace redgiant

