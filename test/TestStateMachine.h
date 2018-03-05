#ifndef TESTSTATEMACHINE_H
#define TESTSTATEMACHINE_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class CTestStateMachine : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CTestStateMachine);
  CPPUNIT_TEST ( test_match_indexes );
  CPPUNIT_TEST_SUITE_END();

protected:
  void test_match_indexes(void);

private:

};

#endif // TESTSTATEMACHINE_H

