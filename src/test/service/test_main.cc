#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "utils/logger.h"
#include "utils/logger-inl.h"

using namespace redgiant;

int main( int argc, char **argv)
{
  init_logger("../resources/log4cxx.xml");

  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest(registry.makeTest());
  // return 0 if successful
  return (int)!runner.run();
}
