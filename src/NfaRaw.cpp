#include "NfaRaw.h"

#include <iostream>
#include <algorithm>
#include <stack>

CF_NFA::CNfaRaw::CNfaRaw() :
  m_last_error(0),
  m_match(NULL)
{  
}

CF_NFA::CNfaRaw::~CNfaRaw()
{
  for (size_t i = 0; i < m_lst_states.size(); ++i)
    if (m_lst_states[i] != NULL)
      delete m_lst_states[i];
  if (m_match) delete m_match;
}
////////////////////////////////////////////////////////////////////////////

int CF_NFA::CNfaRaw::from_postfix(CNfaRaw &nfa_raw, const std::vector<int> &src_lst)
{
  aux_frag_t e1, e2, e;
  nfa_aux_state_t *tmp_st;
  std::stack<aux_frag_t> stack;
  int32_t index = 0;

  if(src_lst.empty())
    return -1;

#define push(state) (stack.push(state))
#define pop() stack.top(); stack.pop()

  for(auto src = src_lst.cbegin(); src != src_lst.cend(); src++) {
    switch(*src) {
      default:        
        tmp_st = new NfaAuxState(CRe2Postfix::from_escape(*src), NULL, NULL, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        push(AuxFrag(tmp_st, aux_nfa_states_ptr_list(&tmp_st->out)));
        break;
      case '.':
        tmp_st = new NfaAuxState(SS_DOT, NULL, NULL, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        push(AuxFrag(tmp_st, aux_nfa_states_ptr_list(&tmp_st->out)));
        break;
      case CRe2Postfix::SS_CONCAT:	/* catenate */
        e2 = pop();
        e1 = pop();
        patch(e1.out, e2.start);
        push(AuxFrag(e1.start, e2.out));
        break;
      case '|':	/* alternate */
        e2 = pop();
        e1 = pop();
        tmp_st = new NfaAuxState(SS_SPLIT, e1.start, e2.start, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        push(AuxFrag(tmp_st, append(e1.out, e2.out)));
        break;
      case '?':	/* zero or one */
        e = pop();
        tmp_st = new NfaAuxState(SS_SPLIT, e.start, NULL, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        push(AuxFrag(tmp_st, append(e.out, aux_nfa_states_ptr_list(&tmp_st->out1))));
        break;
      case '*':	/* zero or more */
        e = pop();
        tmp_st = new NfaAuxState(SS_SPLIT, e.start, NULL, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        patch(e.out, tmp_st);
        push(AuxFrag(tmp_st, aux_nfa_states_ptr_list(&tmp_st->out1)));
        break;
      case '+':	/* one or more */
        e = pop();
        tmp_st = new NfaAuxState(SS_SPLIT, e.start, NULL, ++index);
        nfa_raw.m_lst_states.push_back(tmp_st);
        patch(e.out, tmp_st);
        push(AuxFrag(e.start, aux_nfa_states_ptr_list(&tmp_st->out1)));
        break;
    }
  }

  e = pop();
  if(stack.size() != 0)
    return -2;

  nfa_raw.m_match = new NfaAuxState(SS_MATCH, NULL, NULL, ++index);
  patch(e.out, nfa_raw.m_match);

  tmp_st = new NfaAuxState(SS_START, e.start, NULL, 0);
  nfa_raw.m_lst_states.push_back(tmp_st);

  std::sort(nfa_raw.m_lst_states.begin(),
            nfa_raw.m_lst_states.end(),
            NfaAuxStateComparator());
  return nfa_raw.m_last_error;
#undef pop
#undef push
}
////////////////////////////////////////////////////////////////////////////

static const char* SpecialStatesStr[] = {"dot", "split", "match", "start"};
void CF_NFA::CNfaRaw::print_signal(int signal)
{
  switch (signal) {
    case SS_SPLIT:
    case SS_MATCH:
    case SS_START:
    case SS_DOT:
      std::cout << SpecialStatesStr[signal + SS_MAX];
      break;
    default:
      std::cout << (char)signal;
      break;
  }
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfaRaw::print_nfa_raw() const
{
  typedef std::vector<nfa_aux_state_t*>::const_iterator c_iter;
  for (c_iter i = m_lst_states.begin(); i != m_lst_states.end(); ++i) {
    std::cout << "state : " << (*i)->index;// << " addr : " << *i ;//<< std::endl;
    for (c_iter j = m_lst_states.begin(); j != m_lst_states.end(); ++j) {
      if (*j == (*i)->out) std::cout << " out : " << (*j)->index;
      if (*j == (*i)->out1) std::cout << " out1 : " << (*j)->index;      
    }
    std::cout << " signal : "; print_signal((*i)->signal); std::cout << std::endl;
  }
  std::cout << std::endl;
}
////////////////////////////////////////////////////////////////////////////
