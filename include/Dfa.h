#ifndef CDFA_H
#define CDFA_H

#include <vector>
#include "Commons.h"
#include "Nfa.h"

namespace CF_NFA {
  class CDfa
  {  
  private:
    int m_n_transitions;
    std::vector<int> *m_transitions_arr;
    bool m_is_final;
    std::vector<int> m_number;

    static inline bool vector_contains_dfa_with_number(const std::vector<CDfa>& lst,
                                                       const std::vector<int>& number) {
      for (auto i = lst.cbegin(); i != lst.cend(); ++i)
        if (CCommons::compare_vectors<int>(i->m_number, number) == 0) return true;
      return false;
    }

    void add_transition(const dct_transitions_t &trans, const alphabet_set_t &alp);    
    void add_to_transition(int index, const int num);
    void init_transitions_arr(void);

    class DfaByNumberComparator {
    public:
      bool operator()(const CDfa& s1, const CDfa& s2) {
        return CCommons::compare_vectors(s1.m_number, s2.m_number) <= 0;
      }
    };

  public:
    CDfa(size_t N, int num, bool is_final);
    CDfa(size_t N, const std::vector<int> &number);
    CDfa(CDfa&& dfa);
    CDfa& operator=(const CDfa& dfa) = default;
    ~CDfa(void);

    int n_transitions(void) const {return m_n_transitions;}
    const std::vector<int>& number(void) const {return m_number;}
    bool is_final(void) const {return m_is_final;}
    const std::vector<int>* transitions(void) const {return m_transitions_arr;}

    ///
    /// \brief DfaTableFromNfa
    /// \param nfa_table - input nfa table
    /// \param alp - alphabet
    /// \return vector of Dfa states
    ///
    static std::vector<CDfa> DfaTableFromNfa(const std::vector<CNfa>& nfa_table, const alphabet_set_t &alp);

    ///
    /// \brief RenameStatesInDfaTable
    /// Change numbers of states
    /// \param dfa_tbl
    ///
    static void RenameStatesInDfaTable(std::vector<CDfa> &dfa_tbl);
    static void PrintDebugInfo(std::vector<CDfa>& dfa_table, const alphabet_set_t& alp);
  };

}

#endif // CDFA_H
