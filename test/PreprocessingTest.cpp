#include <string.h>
#include "PreprocessingTest.h"
#include "Commons.h"
#include "Re2Postfix.h"

void CPreprocessingTest::test_full_preprocessing()
{
#define FP_TEST(x, y) CPPUNIT_ASSERT(int_test_full_preprocessing(x, y))
  //simplest
  FP_TEST("a", "a");
  FP_TEST("ab", "ab");
  //round
  FP_TEST("(a)", "(a)");
  FP_TEST("(ab)", "(ab)");
  FP_TEST("ab(c", "ab(c");
  FP_TEST("ab((c", "ab((c");
  FP_TEST("ab(c((", "ab(c((");
  FP_TEST("ab(c)", "ab(c)");
  FP_TEST("(ab(c))", "(ab(c))");
  FP_TEST("(ab+))", "(ab+))");
  FP_TEST("((a+b)*(cd)", "((a+b)*(cd)");
  //square
  FP_TEST("[ab]", "(a|b)");
  FP_TEST("[(ab]", "(\\(|a|b)");
  FP_TEST("ab[c", "ab[c");
  FP_TEST("ab[c]", "ab(c)");
  FP_TEST("ab[cd]", "ab(c|d)");
  FP_TEST("ab]c", "ab]c");
  FP_TEST("[a-c]", "(a|b|c)");
  FP_TEST("[^\x01-ac-\x7f]", "(b)");
  FP_TEST("[^\x01-ad-\x7f]", "(b|c)");
  //figured
  FP_TEST("a{1}", "((a))");
  FP_TEST("a{", "a{");
  FP_TEST("a{}", "a{}");
  FP_TEST("a{1,3}", "((a)(a)?(a)?)");
  FP_TEST("a{3,}", "((a)(a)(a)+)");
  FP_TEST("a{,3}", "((a)?(a)?(a)?)");
  FP_TEST("a{,3", "a{,3");
  FP_TEST("a{{2}", "a(({)({))");
  FP_TEST("(a*){2}", "(((a*))((a*)))");
  FP_TEST("a*{2}", "((a*)(a*))");
  //full
  FP_TEST("a+[b-d]{2}", "a+(((b|c|d))((b|c|d)))");
#undef FP_TEST
}

/////////////////////////////////////////////////////////////////////////

bool CPreprocessingTest::int_test_full_preprocessing(const std::string &src, const std::string &dst)
{
  std::string res = CRe2Postfix::regex2posix(src.c_str());
  return res == dst;
}
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void CPreprocessingTest::test_postfix()
{
  //postfix test
#define P_TEST(x, y) CPPUNIT_ASSERT(int_test_postfix(x, y))
  int res0[] = {'a'};
  P_TEST("a", res0);

  int res1[] = {'a', 'b', CRe2Postfix::SS_CONCAT, '\0'};
  P_TEST("ab", res1);

  int res2[] = {'a', '+', 'c', CRe2Postfix::SS_CONCAT, 'b', '|', '\0'};
  P_TEST("a+c|b", res2);

  int res3[] = {'a', '+', 'b', 'c', CRe2Postfix::SS_CONCAT, 'd', '|', CRe2Postfix::SS_CONCAT, '\0'};
  P_TEST("a+(bc|d)", res3); //"a+bc&d|&"
#undef P_TEST
}

bool CPreprocessingTest::int_test_postfix(const std::string& src, const int *dst)
{
  int res;
  alphabet_set_t alp;
  ubyte_t *buff = new ubyte_t[src.size()+1];
  memcpy(buff, src.c_str(), src.size());
  buff[src.size()] = 0;
  std::vector<int> r2p = CRe2Postfix::regex2postfix(buff, res, alp);
  delete[] buff;

  if (res != 0) return false;
  for (auto i = r2p.begin(); i != r2p.end(); ++i, ++dst) {
    if (*i != *dst) return false;
  }
  return true;
}
/////////////////////////////////////////////////////////////////////////
