#include <string.h>
#include <iostream>
#include "Commons.h"
#include "Re2Postfix.h"
#include "NfaRaw.h"
#include "Nfa.h"
#include "Dfa.h"
#include "DfaCell.h"
#include "SrmApplication.h"
#include <chrono>

using namespace CF_NFA;

///
/// \brief The regex_test_t struct
/// Uses in debug_testing() method for storing pattern string and input data
struct regex_test_t {
  const char* re_pattern;
  const char* str_test;
};

///
/// \brief print_match
/// \param start - start index of match
/// \param end - end index of match
/// \param arg - additional information
/// Callback function. Calls when pattern match detected
void print_match(int start, int end, void* arg) {
  UNUSED_ARG(arg);
  std::cout << std::endl << "MATCH!!! s : " << start << ", m : " << end;
}
/////////////////////////////////////////////////////////////////////////

///
/// \brief debug_testing
/// \return 0 or error code if something happens
/// This function tries to find matches, specified by pattern, in input data.
/// See regex_test_t for more information
int debug_testing() {
  char *buff = new char[2048];
  memset(buff, 0, 2048);
  regex_test_t test[] = {
//    {"a", "ba"},
//    {"a\\){3}", "a)) a)))a)) aaa aaaaa aaaaaa"},
//    {"(ca){,3}", "cacacaca caca ca"},
//    {"(ca){2,}", "cacacaca caca ca"},
//    {"fo*b|c[a-c]+b", "fooc fob cab cbb fooobar cb ccc"},
//    {"c(a|b)+b", "cab cbb cb"},
//    {"(a|b)*bb", "bbb abbb bb a "},
//    {"a\\+b", "a+aa+bbbabbab+aa+ba+bbba"},
//    {"(a|b)*bbc", "bbc abbc bb ca aabbbbc "},
//    {"(ab)+", "ba ab bb aa ababab abab ab "},
//    {"b(na)*c", "bnac bc nanana aannnaaak"},
//    {"(a|b)|c?b", "a r ab cb c"},
//    {"cadcafctr|padpadpafttr", "cadcafcadcadcafctr cacacadcadpadpadpadpafttr "},
//    {"cadcadcabf", "cadcadcadcadcabf"},
//    {"cadcafctr", "cadcadcafcadcafctr"},
//    {"a.*b[0-9]", "ab2 \na33b3\n"},
//    {"a[a-c]*c[0-2][a-c0-2]*c", "abbc2abc"},
//    {"[^bc]*", "aabc"},
//    {"..", "aabcd ii4"},
//    {"[:alnum:]", "hello$world##!!how are you"},
//    {"[:alpha:]", "bir 2 uch 4 besh"},
//    {"[:ascii:]", "abcd e"},
//    {"[:blank:]", "hello world !!!"},
//    {"[:cntrl:]", "hello\nworld !!!"},
//    {"[:digit:]-[:digit:]", "55-654-44-e"},
//    {"[:graph:]", "hello \nworld"},
//    {"[:lower:]", "HELLO world!"},
//    {"[:print:]", "hello \nworld"},
//    {"[:punct:]", "a[b!c\"d#e$f%g&h'i(j)k*l+m,n\\o-p.q/r:s;t<u=v>w?x@y]z^1_2`3{4|5}6~7]8"},
//    {"[:space:]", "arg1 arg2\t   "},
//    {"[:upper:]", "HELLO world!"},
//    {"[:word:]", "hell_world implementation"},
//    {"[:xdigit:]", "FF23E hhh dd12"},
//    {"c\\d", "ab3 cc40"},
//    {"c\\D", "ab3 cc40"},
//    {"\\s", "be bebe\t be\n"},
//    {"\\S", "be bebe\t be\n"},
//    {"\\w", "hello_world megamutant edition"},
//    {"\\W", "hello_world megamutant edition"},
//  #ifndef DFA_CELL_16
//    {"...", "aabcd ii4"},
//    {"(1|2|3)(1|2|3)2", "131322222"},
//    {"............", "012345678901"},
//    {"a+++++++++++++++++++++++++++a+", "bbfaa"},
//    {"c.*c.*c12345678910", "c012cbcbbbddc12345678910111213141516171819202122"},
//    {"c(1|2|3|c)*c(1|2|3|c)*c123","c12cc123c123"},
//    {"((1|2|3)(1|2|3)(1|2|3))*3123", "3123"},
//    {"sdk", "sdksdk"},
//    {"(a|b|c)(a|b|c)(a|b|c)" , "abcabbacc"},
//    {"(......................................................................)*", "a"},
//  #endif
//    {"c((abcd)|[a-c])c((abcd)|[a-c])c1*", "cacabcdc1111c"},
//    {"c[a-c]c[a-c]c1", "cacbcacac1"},
//    {"c.*a", "cacacabbcabc"},
//    {"c[a-c]*a", "cbbcba"},
//    {"c(a|b)dc(a|b)dcef", "cadcbdcbdcef"},
//    {"[a-d][a-d][a-d][a-d]", "abcdabcc"},
//    {"c.c.c1", "cacacacbc1c1"},
//    {"([a-e][a-e])|ccdeb", "ccdeccdeb"},
    {".{1000}", "cavb"},
    {NULL, NULL}
  };

  int res;
  for (size_t i = 0; test[i].re_pattern; ++i) {
    std::cout << std::endl << "*********************************" << std::endl;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    std::chrono::duration<double> time;
    start = std::chrono::high_resolution_clock::now();
    CDfaTable* dfa = CDfaTable::dfa_table_from_regexp(test[i].re_pattern, res);
    end = std::chrono::high_resolution_clock::now();
    time = end-start;
    std::cout << "construction time : " << time.count() << std::endl;

    if (res != 0) {
      std::cout << "debug_testing error. re : " << test[i].re_pattern << " . res_code : " << res << std::endl;
      if (dfa) delete dfa;
      continue;
    }

    if (dfa) {
      dfa->run_state_machine(test[i].str_test, test[i].re_pattern, print_match, NULL) ;
      std::cout << std::endl << "Press enter for next" << std::endl;
      std::cin.get();
    }
    if (dfa) delete dfa;
  } //for
  delete [] buff;
  return 0;
}
////////////////////////////////////////////////////////////////////////////



///
/// \brief main
/// \param argc . If 1 - we use re-dfa tool with tudoop server. else it can be "-debug" or input_file_name.
/// if argv[1] == "-debug" debug_testing method will be called.
/// else argv[1] will be used as input file name.
/// \param argv
/// \return
/// The entry point of the re-dfa tool.
int main(int argc, char* argv[]) {
#ifdef DFA_CELL_16
  std::cout << "Re-dfa 16-bit started!" << std::endl;
#else
  std::cout << "Re-dfa 32-bit started!" << std::endl;
#endif
  if (argc == 2) {
    if (strcmp(argv[1], "-debug") == 0) {
      CAppConfig::Instance()->Init(true);
      debug_testing();
      return 0;
    }
    else {
      CAppConfig::Instance()->Init();
      if (CAppConfig::Instance()->use_hardware()) {
        return SrmApplication::run(argv[1]);
      }
      else {
        int res(0);
        CDfaTable* dfa = CDfaTable::dfa_table_from_regexp(CAppConfig::Instance()->re_pattern().c_str(), res);
        if (res) {
          printf("Error creating dfa : %d\n", res);
          return -1;
        }
        dfa->run_state_machine_file(argv[1]);
        delete dfa;
        return 0;
      }
    }
  }
  else {
    CAppConfig::Instance()->Init();
    return SrmApplication::run(NULL); //we use network for receiving input file.
  }
}
////////////////////////////////////////////////////////////////////////////
