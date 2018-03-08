#include "Dfa.h"

CF_NFA::CDfa::CDfa(size_t N, int num, bool is_final) :
  m_n_transitions(N),
  m_transitions_arr(NULL),
  m_is_final(is_final),
  m_number(1, num)
{
}

CF_NFA::CDfa::CDfa(size_t N, const std::vector<int>& number) :
  m_n_transitions(N),
  m_transitions_arr(NULL),
  m_is_final(false),
  m_number(std::move(number))
{
}

CF_NFA::CDfa::CDfa(CF_NFA::CDfa &&dfa) :
  m_n_transitions(dfa.m_n_transitions),
  m_transitions_arr(std::move(dfa.m_transitions_arr)),
  m_is_final(dfa.m_is_final),
  m_number(dfa.m_number)
{
  dfa.m_transitions_arr = NULL;
}

CF_NFA::CDfa::~CDfa()
{
  if (m_transitions_arr) {
    delete[] m_transitions_arr;
    m_transitions_arr = NULL;
  }
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfa::add_transition(const CF_NFA::dct_transitions_t &trans, const alphabet_set_t &alp)
{
  int ii = 0;
  for (auto i = alp.begin(); i != alp.end(); ++i, ++ii) {
    if (trans.find(*i) == trans.end()) continue;
    for (auto j = trans.at(*i).begin(); j != trans.at(*i).end(); ++j) {
      this->add_to_transition(ii, *j);
    }
  } // for i
}

void CF_NFA::CDfa::add_to_transition(const int index, const int num)
{
  CCommons::add_elem_to_sorted_vector<int>(m_transitions_arr[index], num);
}
/////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfa::init_transitions_arr()
{
  if (m_transitions_arr) delete[] m_transitions_arr;
  m_transitions_arr = new std::vector<int>[m_n_transitions];
}
////////////////////////////////////////////////////////////////////////////

std::vector<CF_NFA::CDfa> CF_NFA::CDfa::DfaTableFromNfa(const std::vector<CF_NFA::CNfa> &nfa_table,
                                                        const alphabet_set_t& alp)
{
  std::vector<CF_NFA::CDfa> result;
  result.emplace_back(alp.size(), nfa_table[0].number(), nfa_table[0].is_final());
  std::vector<int> tmp_lst(8); //ahtung!!

  //fill numbers
  for (size_t current = 0; current < result.size(); ++current) {
    for (auto ch = alp.cbegin(); ch != alp.cend(); ++ch) {
      tmp_lst.clear(); //without changing capacity
      for (auto i = result[current].m_number.begin(); i != result[current].m_number.end(); ++i) {
        result[current].m_is_final |= nfa_table[*i].is_final();

        if (nfa_table[*i].dct_transitions().find(*ch) == nfa_table[*i].dct_transitions().end())
          continue;

        for (auto k = nfa_table[*i].dct_transitions().at(*ch).begin();
             k != nfa_table[*i].dct_transitions().at(*ch).end(); ++k) {
          CCommons::add_elem_to_sorted_vector<int>(tmp_lst, *k);
        } //for k
      } //for number size

      if (tmp_lst.empty()) continue;
      if (vector_contains_dfa_with_number(result, tmp_lst)) continue;
      result.emplace_back(alp.size(), tmp_lst);
    } //for j
  } //for current

  std::sort(result.begin(), result.end(), DfaByNumberComparator());

  //fill transitions . I think here dot should be processed. and other symbol groups too.
  for (auto cur = result.begin(); cur != result.end(); ++cur) {
    cur->init_transitions_arr();
    for (auto i = cur->m_number.begin(); i != cur->m_number.end(); ++i) {
      cur->add_transition(nfa_table[*i].dct_transitions(), alp);
    }
  }
  return result;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfa::RenameStatesInDfaTable(std::vector<CF_NFA::CDfa> &dfa_tbl)
{  
  register uint32_t f, l, m;
  register int cmp;

  for (auto i = dfa_tbl.begin(); i != dfa_tbl.end(); ++i) {
    for (int k = 0; k < i->m_n_transitions; ++k) {
      if (i->m_transitions_arr[k].empty()) continue;

      l = (uint32_t) dfa_tbl.size();
      f = 0;
      while (f < l) {
        m = (f + l) >> 1;
        cmp = CCommons::compare_vectors<int>(i->m_transitions_arr[k], dfa_tbl[m].m_number);
        if (cmp <= 0) l = m;
        else f = m+1;
      }
      i->m_transitions_arr[k][0] = l;
    }
  }

  register int st_num = 0;
  for (auto i = dfa_tbl.begin(); i != dfa_tbl.end(); ++i, ++st_num)
    i->m_number[0] = st_num;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfa::PrintDebugInfo(std::vector<CF_NFA::CDfa> &dfa_table, const alphabet_set_t &alp)
{
  //debug print. ok
  std::cout << std::endl;
  for (auto i = dfa_table.cbegin(); i != dfa_table.cend(); ++i) {

    std::cout << "st : ";
    for (size_t b = 0; b < i->m_number.size(); ++b)
      std::cout << i->m_number[b] << ",";
    std :: cout << (i->is_final() ? "F" : "") << std::endl;

    for (size_t j = 0; j < alp.size(); ++j) {
      std::cout << (char)alp[j] << " -> ";
      for (auto k = i->m_transitions_arr[j].begin(); k != i->m_transitions_arr[j].end(); ++k) {
        std::cout << *k << " ";
      }
      std::cout << "; " << std::endl;
    }
  }
  std::cout << std::endl;
}
////////////////////////////////////////////////////////////////////////////
