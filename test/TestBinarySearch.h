#ifndef CTESTBINARYSEARCH_H
#define CTESTBINARYSEARCH_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

class CTestBinarySearch : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CTestBinarySearch);
  CPPUNIT_TEST( test_add_to_empty_arr );
  CPPUNIT_TEST( test_add_0_to_1 );
  CPPUNIT_TEST( test_add_1_to_0 );
  CPPUNIT_TEST( test_add_0_to_0 );
  CPPUNIT_TEST( test_add_3_values );
  CPPUNIT_TEST( test_2_sorted_vectors );

  CPPUNIT_TEST_SUITE_END();

public:
protected:
  void test_add_to_empty_arr(void);
  void test_add_0_to_1(void);
  void test_add_1_to_0(void);
  void test_add_0_to_0(void);
  void test_add_3_values(void);
  void test_2_sorted_vectors(void);

private:
};

#endif // CTESTBINARYSEARCH_H
