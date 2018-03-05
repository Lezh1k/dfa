#include "Commons.h"
#include "PreprocessingTest.h"
#include "TestBinarySearch.h"
#include <cppunit/ui/text/TestRunner.h>

int main(int argc, char* argv[]) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(CTestBinarySearch::suite());
  runner.addTest(CPreprocessingTest::suite());

  runner.run();
  return 0;
}
/////////////////////////////////////////////////////////////////////////
