#ifndef CCOMMONS_H
#define CCOMMONS_H

#define UNUSED_ARG(x) ((void)x)

#include <stddef.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#include <vector>
#include <algorithm>
#include <string>
#include <stdint.h>

typedef std::vector<int32_t> alphabet_set_t;

class CCommons {
private:
  CCommons();
  CCommons(const CCommons&);

public:

  enum {POSSIBLE_BYTE_VALUES_COUNT = 0x100};

  static void add_to_alphabet(int32_t signal, std::vector<int32_t>& alp) {
    add_elem_to_sorted_vector<int32_t>(alp, signal);
  }

  static bool is_numeric(char c) {
    return c >= '0' && c <= '9';
  }
  ////////////////////////////////////////////////////////////////////////////

  template<class T> static void add_elem_to_sorted_vector(std::vector<T>& vector, T num) {
    if (vector.empty() || num > vector[vector.size()-1]) {
      vector.push_back(num);
      return;
    }
    if (num < vector[0]) {
      vector.insert(vector.begin(), num);
      return;
    }
    //binary search.
    register uint32_t f, l, m; //first, last, middle.
    f = 0; l = vector.size();
    while (f < l) {
      m = (f + l) >> 1;
      if (num == vector[m]) return;
      if (num < vector[m]) l = m;
      else f = m+1;
    }
    if (vector[l] == num)
      return;
    vector.insert(vector.begin()+l, num);
  }
  ////////////////////////////////////////////////////////////////////////////

  //use this because 95% of vectors in current project are not sse aligned
  //and operator== can't be optimized with sse instructions.
  template<class T> static int compare_vectors(const std::vector<T>& arg1, const std::vector<T>& arg2) {
    if (arg1.size() != arg2.size()) return arg1.size() < arg2.size() ? -1 : 1;
    auto i1 = arg1.cbegin();
    auto i2 = arg2.cbegin();
    for (; i1 != arg1.cend(); ++i1, ++i2)
      if (*i1 != *i2) return *i1 < *i2 ? -1 : 1;
    return 0;
  }
  ////////////////////////////////////////////////////////////////////////////

  template<class T> static int binary_search(const T* arr, T elem, uint32_t N) {
    if (arr==NULL || N==0) return -1;
    if (arr[0]>elem || arr[N-1]<elem) return -1;
    register uint32_t f=0, m;
    while (f < N) {
      m = (f + N) >> 1;
      if (elem <= arr[m]) N = m;
      else f = m+1;
    }
    return arr[N]==elem ? N : -1;
  }
  ////////////////////////////////////////////////////////////////////////////

  static std::string& ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends
  static std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }

  static std::string &to_lower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
  }
  //////////////////////////////////////////////////////////////////////////

};

#endif // CCOMMONS_H
