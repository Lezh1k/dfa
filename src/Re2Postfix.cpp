#include <cstring>
#include <iostream>
#include <string>
#include <stack>
#include <algorithm>
#include "Commons.h"
#include "Re2Postfix.h"

////////////////////////////////////////////////////////////////////////////

const char* CRe2Postfix::m_escapes_str = "&+\\()?*|^${}[]-.";
const int CRe2Postfix::m_escapes_len = strlen(CRe2Postfix::m_escapes_str)+CCommons::POSSIBLE_BYTE_VALUES_COUNT;
////////////////////////////////////////////////////////////////////////////

struct paren_t {
  int nalt;
  int natom;
  paren_t() : nalt(0), natom(0){}
};

std::vector<int> CRe2Postfix::regex2postfix(uint8_t *re, int &res_code, alphabet_set_t &alphabet) {

#define RETURN_ERR(err_code) do {\
  res_code = err_code; \
  return std::vector<int>(0); \
} while (0);

  //n alternatives , n atoms
  //atom is any symbol except ()|*?+
  int nalt, natom;
  std::vector<int> res;
  int c; //tmp char for escape sequence
  //parentheses
  paren_t p;
  std::stack<paren_t> st_paren;
  bool dot_processed = false;
  //in curent parentheses
  nalt = natom = 0;
  for(;*re;re++){
    switch(*re){
      case '\\' : //maybe we have an escape? if we don't, go to default.
        if (*(re+1) && ((c = to_escape(*(re+1))) != *(re+1))) {
          CCommons::add_to_alphabet(from_escape(c), alphabet);// alphabet.insert(*re);
          if(natom > 1){
            --natom;
            res.push_back(SS_CONCAT);
          }
          res.push_back(c);
          natom++;
          ++re;
          break;
        }
      case '.' :
        if (!dot_processed) {
          for (int i = 0; i < CCommons::POSSIBLE_BYTE_VALUES_COUNT; ++i) {
            CCommons::add_to_alphabet(i, alphabet);
          }
          dot_processed = true;
        }
      default: //any character, except escape, ()|+?*
        CCommons::add_to_alphabet(*re, alphabet);// alphabet.insert(*re);
        if(natom > 1){
          --natom;
          res.push_back(SS_CONCAT);
        }
        res.push_back(*re);
        natom++;
        break;

      case '(':
        if(natom > 1){
          --natom;
          res.push_back(SS_CONCAT);
        }

        p.nalt = nalt;
        p.natom = natom;
        st_paren.push(p);
        nalt = 0;
        natom = 0;
        break;

      case '|':
        if(natom == 0)
          RETURN_ERR(R2P_WRONG_REGEX); //we havent operands here
        while(--natom > 0)
          res.push_back(SS_CONCAT);
        nalt++;
        break;

      case ')':
        if(st_paren.empty())
          RETURN_ERR(R2P_WRONG_PAREN);
        if(natom == 0)
          RETURN_ERR(R2P_WRONG_REGEX);

        while(--natom > 0)
          res.push_back(SS_CONCAT);
        for(;nalt > 0; --nalt)
          res.push_back('|');
        p = st_paren.top();
        st_paren.pop();
        nalt = p.nalt;
        natom = p.natom;
        natom++;
        break;

      case '*':
      case '+':
      case '?':
        if(natom == 0)
          RETURN_ERR(R2P_WRONG_REGEX);
        res.push_back(*re);
        break;
    }
  }

  if(!st_paren.empty())
    RETURN_ERR(R2P_WRONG_PAREN);

  while(--natom > 0)
    res.push_back(SS_CONCAT);

  for(; nalt > 0; nalt--)
    res.push_back('|');
  res_code = R2P_SUCCESS;
  return res;
#undef RETURN_ERR
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::get_minus_class(char c1, char c2)
{
  if (c1 > c2) return "-";
  std::string result = "";
  for (int c = c1; c <= c2; ++c)
    result += (char)c;
  return result;
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::get_symbol_class(const std::string &str, bool neg)
{
  std::string result = "";
  if (!neg) {

    for (size_t i = 0; i < str.size(); ++i) {
      if (result.find_first_of(str[i]) != std::string::npos &&
          (str[i] != '\\' && str[i] != '|') ) continue;
      if (is_escape(str[i]))
        result += '\\';
      result += str[i] + std::string("|");
    }
    result[result.size()-1] = ')';
    result = "(" + result;
    return result;
  }

  for (size_t i = 1; i < 0x7F; ++i) { //we don't need 0x00 in result string
    if (str.find_first_of((char)i) != std::string::npos) continue;
    if (is_escape(i)) result += '\\';
    result += (char)i + std::string("|");
  }

  result = "(" + result;
  result[result.size()-1] = ')';
  return result;
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::from_square(const char *re, int &slen)
{
  std::string result = "";
  bool neg = re[0] == '^';
  slen = neg ? 2 : 1;
  size_t start = neg ? 1 : 0;
  for (size_t i = start; i < strlen(re); ++i, ++slen) {
    switch (re[i]) {
      case ']' :
        return get_symbol_class(result, neg);
      case '-' :
        if (i == start || re[i+1] == ']') {
          result += '-';
          break;
        }
        result += get_minus_class(re[i-1], re[i+1]);
        break;
      case '\\':
        ++i; ++slen;
      default:
        result += re[i];
        break;
    }
  }
  slen = 0;
  return "[";
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::from_figured(const char *re, const std::string &token, int &len)
{
  std::string str_analize = "";
  std::string str_res = "";
  size_t n_comas = 0;
  size_t i;
  for (i = 0; i < strlen(re); ++i) {
    if (re[i] == '}') break;
    if (re[i] == ',') ++n_comas;
    if (re[i] != ',' && !CCommons::is_numeric(re[i])) {
      len = 1;
      return "{";
    } else
      str_analize += re[i];
  }

  if (i == strlen(re) && re[i-1] != ']') { len = 1; return "{";}
  if (str_analize.size() == 0){ len = 1; return "{"; }
  if (n_comas > 1){len = 1; return "{";}

  if (n_comas == 0) {
    int arg = std::stoi(str_analize);
    if (arg == 0) {len = 1; return "{";}

    len = str_analize.size() + 2;
    str_res += "(";
    for (int i = 0; i < arg; ++i) {
      str_res += std::string("(") + token + std::string(")");
    }
    str_res += ")";
    return str_res;
  }

  std::string sarg1, sarg2;
  size_t index = str_analize.find_first_of(',');
  sarg1 = str_analize.substr(0, index);
  sarg2 = str_analize.substr(index+1, str_analize.size() - index);
  int arg1 = sarg1.size() > 0 ? std::stoi(sarg1) : 0;
  int arg2 = sarg2.size() > 0 ? std::stoi(sarg2) : 0;
  if (arg1 == 0 && arg2 == 0) {len = 1; return "{";}

  if (arg1 == 0 && arg2 != 0) {
    str_res += "(";
    for (int i = 0; i < arg2; ++i) {
      str_res += "(" + token + ")?";
    }
    str_res += ")";
  }
  else if (arg1 != 0 && arg2 == 0) {
    str_res += "(";
    for (int i = 0; i< arg1-1; ++i) {
      str_res += "(" + token + ")";
    }
    str_res += "(" + token + ")+)";
  }
  else { /*arg1 != 0 && arg2 != 0*/
    str_res += "(";
    for (int i = 0; i < arg1; ++i) {
      str_res += "(" + token + ")";
    }
    for (int i = arg1; i < arg2; ++i) {
      str_res += "(" + token + ")?";
    }
    str_res += ")";
  }

  len = str_analize.size() + 2;
  return str_res;
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::preprocess_1(const char *re)
{
  static std::pair<std::string, std::string> dct[] = {
    {"[:alnum:]", "[a-zA-Z0-9]"},
    {"[:alpha:]", "[a-zA-Z]"},
    {"[:ascii:]", "[\x01-\x7F]"},
    {"[:blank:]", "[ \t]"},
    {"[:cntrl:]", "[\x01-\x1F\x7F]"},
    {"[:digit:]", "[0-9]"},
    {"[:graph:]", "[\x21-\x7E]"},
    {"[:lower:]", "[a-z]"},
    {"[:print:]", "[\x20-\x7E]"},
    {"[:punct:]", "[!\"#$%&'()*+,\\-./:;<=>?@[\\]\\\\^_`{|}~]"},
    {"[:space:]", "[ \t\r\n\v\f]"},
    {"[:upper:]", "[A-Z]"},
    {"[:word:]", "[A-Za-z0-9_]"},
    {"[:xdigit:]", "[A-Fa-f0-9]"},
    {"\\d", "[0-9]"},
    {"\\D", "[^0-9]"},
    {"\\s", "[ \f\n\r\t\v]"},
    {"\\S", "[^ \f\n\r\t\v]"},
    {"\\w", "[A-Za-z0-9_]"},
    {"\\W", "[^A-Za-z0-9_]"},
    {"", ""},
  };

  std::string res(re);
  for (size_t i = 0; dct[i].first != ""; ++i) {
    size_t index = res.find(dct[i].first, 0);
    while(index != std::string::npos) {
      if (index == 0) {
        res = res.replace(index, dct[i].first.size(), dct[i].second);
        index = res.find(dct[i].first, index+1);
        continue;
      }
      int n_esc = 0;
      while((int)index-n_esc-1 >=0 && res[index-n_esc-1] == '\\')
        ++n_esc;
      if (n_esc % 2 == 0) {
        res = res.replace(index, dct[i].first.size(), dct[i].second);
      }
      index = res.find(dct[i].first, index+1);
    }
  }

  return res;
}
/////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::preprocess_2(const char *re)
{
  std::string result = "";
  std::stack<std::string> st_tokens;
  std::stack<std::string> reverse;
  std::string last_token;
  int res_len = 0;
  int flen, slen;

  for (size_t i = 0; i < strlen(re); ++i) {
    slen = flen = 0;
    last_token = ""; //empty string;
    switch (re[i]) {
      case '(':
        last_token = from_round(&re[i+1], res_len);
        i += res_len+1;
        break;
      case '{':
        if (st_tokens.empty()) {
          last_token += '{';
          break;
        }
        last_token = st_tokens.top();
        last_token = from_figured(&re[i+1], last_token, flen);
        if (flen != 1) st_tokens.pop();
        i += flen-1;
        break;
      case '[':
        last_token = from_square(&re[i+1], slen);
        i += slen;
        break;
      case '+':
      case '?':
      case '*':
        last_token = st_tokens.top();
        st_tokens.pop();
        last_token += re[i];
        break;
      case '\\':
        if (re[i+1]) {
          last_token += '\\';
          ++i;
        }
      default:
        last_token += re[i];
        break;
    } //switch
    st_tokens.push(last_token);
  } //for

  while (!st_tokens.empty()) {
    reverse.push(st_tokens.top());
    st_tokens.pop();
  }
  while (!reverse.empty()) {
    result += reverse.top();
    reverse.pop();
  }
  return result;
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::from_round(const char *re, int& len)
{
  std::string result = "";
  std::stack<std::string> st_tokens;
  std::stack<std::string> reverse;
  std::string last_token;

  int res_len = 0;
  int flen, slen;
  size_t i;

  for (i = 0; i < strlen(re); ++i) {
    slen = flen = 0;
    last_token = ""; //empty string;
    switch (re[i]) {
      case '(':
        last_token = from_round(&re[i+1], res_len);
        i+=res_len+1;
        break;
      case ')':
        result += "(";
        while (!st_tokens.empty()) {
          reverse.push(st_tokens.top());
          st_tokens.pop();
        }
        while (!reverse.empty()) {
          result += reverse.top();
          reverse.pop();
        }
        result += ")";
        len = i;
        return result;
      case '{':
        if (st_tokens.empty()) {
          last_token += '{';
          break;
        }

        last_token = st_tokens.top();
        last_token = from_figured(&re[i+1], last_token, flen);
        if (flen != 1)
          st_tokens.pop();
        i+=flen-1;
        break;
      case '[':
        last_token = from_square(&re[i+1], slen);
        i+= slen;
        break;
      case '+':
      case '?':
      case '*':
        last_token = st_tokens.top();
        st_tokens.pop();
        last_token += re[i];
        break;
      case '\\':
        if (re[i+1]) {
          last_token += '\\';
          ++i;
        }
      default:
        last_token += re[i];
        break;
    } //switch
    st_tokens.push(last_token);
  } //for

  len = -1;
  return "(";
}
////////////////////////////////////////////////////////////////////////////

std::string CRe2Postfix::regex2posix(const char *re)
{
  std::string pre1 = preprocess_1(re);
  std::string res = preprocess_2(pre1.c_str());
  return res;
}
////////////////////////////////////////////////////////////////////////////


bool CRe2Postfix::is_escape(char c)
{
  for (int i=1; m_escapes_str[i]; ++i)
    if (m_escapes_str[i] == c) return true;
  return false;
}
////////////////////////////////////////////////////////////////////////////

int CRe2Postfix::from_escape(int c) {
  return c > -CCommons::POSSIBLE_BYTE_VALUES_COUNT ? c : m_escapes_str[c+m_escapes_len];
}
////////////////////////////////////////////////////////////////////////////

int CRe2Postfix::to_escape(char c) {
  for (int i=1; m_escapes_str[i]; ++i)
    if (m_escapes_str[i] == c) return i-m_escapes_len;
  return c;
}
////////////////////////////////////////////////////////////////////////////
