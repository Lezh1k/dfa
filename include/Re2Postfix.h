#ifndef CRE2POSTFIX_H
#define CRE2POSTFIX_H

#include <string>
#include <vector>
#include "Commons.h"

class CRe2Postfix
{

private:
  CRe2Postfix(void);
  ~CRe2Postfix(void);
  CRe2Postfix(const CRe2Postfix&);
  void operator=(const CRe2Postfix&);
  ////////////////////////////////////////////////////////////////////////////

  static const char* m_escapes_str;
  static const int m_escapes_len;

  ///
  /// \brief from_round
  /// \param re - string betweet ()
  /// \param len - original tokens lenght
  /// \return token from round brackets
  ///
  static std::string from_round(const char *re, int &len);

  ///
  /// \brief get_minus_class
  /// \param c1 - first symbol
  /// \param c2 - last symbol
  /// \return set of alternations between c1 and c2
  ///
  static std::string get_minus_class(char c1, char c2);

  ///
  /// \brief get_symbol_class
  /// This function processes symbol classes like [^a-c] of [a-x] and opens this kind of brackets
  /// ^ -  with negotiation. Means, that all symbols in square brackets will be ignored.
  /// \param str - input string. like a-cE-N
  /// \param neg - should we use symbols in set or ignore those symbols.
  /// \return string of symbol class.
  ///
  static std::string get_symbol_class(const std::string& str, bool neg);

  ///
  /// \brief from_square
  /// This function opens square brackets like this:
  /// [a-c] = (a|b|c)
  /// \param re - input regular expression or part of regular expression (token)
  /// \param slen - original tokens lenght
  /// \return converted string
  ///
  static std::string from_square(const char *re, int &slen);

  ///
  /// \brief from_figured
  /// This function opens token with figured brackets.
  /// Example : a{2,3} = "(aa)|(aaa)".
  /// \param re - input regular expression
  /// \param token - token to process. we use something like recursive descent
  /// and token is part of string, that we are trying to process now. For example
  /// a(bc) has 2 tokens : "a" and "(bc)".
  /// \param len - original tokens lenght
  /// \return converted string
  ///
  static std::string from_figured(const char *re, const std::string &token, int &len);

  ///
  /// \brief is_escape
  /// Escape symbols are "+\\()?*|^${}[]-."
  /// We can escape those symbols by adding \\ before symbol.
  /// \param c - input character
  /// \return true if input character is escape symbol.
  ///
  static bool is_escape(char c);

  ///
  /// \brief CRe2Postfix::preprocess_1
  /// Changes POSIX character sets in source regular expression
  /// \param re - input regular expression
  /// \return regular expression string with POSIX character sets
  ///
  static std::string preprocess_1(const char *re);

  ///
  /// \brief preprocess_2
  /// This function opens brackets. For example [a-c] converts
  /// into (a|b|c), b{2} converts into "(bb)" etc.
  /// \param re - input regular expression
  /// \return converted string
  ///
  static std::string preprocess_2(const char *re);

public:

  //const int CRe2Postfix::SS_CONCAT = -16 - CRe2Postfix::BYTE_INTERVAL;
  static enum { SS_CONCAT = -16-CCommons::POSSIBLE_BYTE_VALUES_COUNT } s;

  typedef enum Re2Post_Errors {
    R2P_SUCCESS = 0,
    R2P_MEMORY_ERROR,
    R2P_WRONG_PAREN,
    R2P_WRONG_REGEX,
  } re_2_post_errors;

  ///
  /// \brief regex2posix
  /// This function changes POSIX character sets to standart character sets.
  /// \param re - input regular expression
  /// \return regular expression without POSIX character sets
  ///
  static std::string regex2posix(const char *re);

  ///
  /// \brief regex2postfix
  /// \param re - input regular expression
  /// \param res_code - result code
  /// \param alphabet - generated alphabet
  /// \return vector of integer values that represents regular expression in postfix form
  ///
  static std::vector<int> regex2postfix(uint8_t *re, int &res_code, alphabet_set_t &alphabet);

  ///
  /// \brief from_escape
  /// \param c - input character
  /// \return code of character.
  ///
  static int from_escape(int c);

  ///
  /// \brief to_escape
  /// \param c - input character
  /// \return negative value if c is escape character
  ///
  static int to_escape(char c);
};

#endif // CRE2POSTFIX_H
