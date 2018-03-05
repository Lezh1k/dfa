#ifndef CPREPROCESSINGTEST_H
#define CPREPROCESSINGTEST_H

#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class CPreprocessingTest : public CppUnit::TestFixture
{  
  CPPUNIT_TEST_SUITE(CPreprocessingTest);
  CPPUNIT_TEST( test_full_preprocessing );
  CPPUNIT_TEST( test_postfix );
  CPPUNIT_TEST_SUITE_END();

public:
protected:
  void test_full_preprocessing(void);
  void test_postfix(void);

private:

  bool int_test_full_preprocessing(const std::string& src, const std::string& dst);
  bool int_test_postfix(const std::string &src, const int *dst);
};

#endif // CPREPROCESSINGTEST_H
