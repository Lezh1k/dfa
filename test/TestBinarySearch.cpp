#include <vector>
#include <algorithm>
#include "TestBinarySearch.h"
#include "Commons.h"

void CTestBinarySearch::test_add_to_empty_arr()
{
  std::vector<int> empty;
  CCommons::add_elem_to_sorted_vector<int>(empty, 0);
  CPPUNIT_ASSERT(empty.size() == 1);
  CPPUNIT_ASSERT(empty[0] == 0);
}
/////////////////////////////////////////////////////////////////////////

void CTestBinarySearch::test_add_0_to_1()
{
  std::vector<int> lst;
  lst.push_back(1);
  CCommons::add_elem_to_sorted_vector<int>(lst, 0);
  CPPUNIT_ASSERT(lst.size() == 2);
  CPPUNIT_ASSERT(lst[0] == 0);
  CPPUNIT_ASSERT(lst[1] == 1);
}
/////////////////////////////////////////////////////////////////////////

void CTestBinarySearch::test_add_1_to_0()
{
  std::vector<int> lst;
  lst.push_back(0);
  CCommons::add_elem_to_sorted_vector<int>(lst, 1);
  CPPUNIT_ASSERT(lst.size() == 2);
  CPPUNIT_ASSERT(lst[0] == 0);
  CPPUNIT_ASSERT(lst[1] == 1);
}
/////////////////////////////////////////////////////////////////////////

void CTestBinarySearch::test_add_0_to_0()
{
  std::vector<int> lst;
  CCommons::add_elem_to_sorted_vector<int>(lst, 0);
  CCommons::add_elem_to_sorted_vector<int>(lst, 0);
  CPPUNIT_ASSERT(lst.size() == 1);
  CPPUNIT_ASSERT(lst[0] == 0);
}
/////////////////////////////////////////////////////////////////////////

void CTestBinarySearch::test_add_3_values()
{
  std::vector<int> lst;
  CCommons::add_elem_to_sorted_vector<int>(lst, 2);
  CCommons::add_elem_to_sorted_vector<int>(lst, 2);
  CCommons::add_elem_to_sorted_vector<int>(lst, 1);
  CCommons::add_elem_to_sorted_vector<int>(lst, 0);
  CCommons::add_elem_to_sorted_vector<int>(lst, 1);
  CCommons::add_elem_to_sorted_vector<int>(lst, 0);
  CPPUNIT_ASSERT(lst.size() == 3);
  CPPUNIT_ASSERT(lst[0] == 0);
  CPPUNIT_ASSERT(lst[1] == 1);
  CPPUNIT_ASSERT(lst[2] == 2);
}
/////////////////////////////////////////////////////////////////////////

void CTestBinarySearch::test_2_sorted_vectors()
{
  std::vector<int> lst1, lst2;
  const int count = 10;
  for (int i = count; i>=0; --i) {
    lst1.push_back(i);
    CCommons::add_elem_to_sorted_vector<int>(lst2, i);
  }
  std::sort(lst1.begin(), lst1.end());
  CPPUNIT_ASSERT(lst1 == lst2);
}
/////////////////////////////////////////////////////////////////////////
