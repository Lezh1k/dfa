#include <iostream>
#include <queue>
#include "Nfa.h"
#include "AppConfig.h"

CF_NFA::CNfa::CNfa() :
  m_number(-1),
  m_is_final(false)
{
}

CF_NFA::CNfa::CNfa(const CF_NFA::CNfa &nfa) :
  m_number(nfa.m_number),
  m_is_final(nfa.m_is_final),
  m_dct_transitions(nfa.m_dct_transitions)
{
}

CF_NFA::CNfa::~CNfa()
{
}
////////////////////////////////////////////////////////////////////////////

std::vector<CF_NFA::CNfa> CF_NFA::CNfa::CreateFromRawNfa(const CF_NFA::CNfaRaw &nfa_raw)
{
  std::vector<CNfa> result(nfa_raw.LstStates().size());
  for (size_t i = 0; i < nfa_raw.LstStates().size(); ++i) {
    if (nfa_raw.LstStates()[i]->signal == CNfaRaw::SS_SPLIT) continue;
    result[i].m_number = nfa_raw.LstStates()[i]->index;
    FillNfaTransitions(result[i], nfa_raw.LstStates()[i]);
  }
  return result;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfa::FillNfaTransitions(CF_NFA::CNfa &nfa, const CF_NFA::CNfaRaw::nfa_aux_state_t *ss)
{
  if (ss->signal == CNfaRaw::SS_MATCH)
    return;

  const CF_NFA::CNfaRaw::nfa_aux_state_t *out[2] = {ss->out, ss->out1}; //we have out and out 1
  for (int i = 0; i < 2; ++i) {
    if (out[i] == NULL) continue;

    //if ss.out.signal == SS_MATCH || ss.out1.signal == SS_MATHC then ss.is_final = 1;
    nfa.m_is_final |= out[i]->signal == CNfaRaw::SS_MATCH;

    switch(out[i]->signal) {
      case CNfaRaw::SS_SPLIT:
        CF_NFA::CNfa::FillNfaTransitions(nfa, out[i]);
        break;
      case CNfaRaw::SS_DOT:
        ProcessDotSignal(nfa, out[i]);
        break;
      default:
        ProcessSimpleSignal(nfa, out[i]);
        break;
    }
  }
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfa::ProcessDotSignal(CF_NFA::CNfa &nfa, const CF_NFA::CNfaRaw::nfa_aux_state_t *out)
{
  if (CAppConfig::Instance()->exclude_carriage_return()) {
    for (int j = 0; j < '\n'; ++j)
      nfa.m_dct_transitions[j].push_back(out->index);
    for (int j = '\n'+1; j < CCommons::POSSIBLE_BYTE_VALUES_COUNT; ++j)
      nfa.m_dct_transitions[j].push_back(out->index);
  } //exclude carriage return
  else {
    for (int j = 0; j < CCommons::POSSIBLE_BYTE_VALUES_COUNT; ++j)
      nfa.m_dct_transitions[j].push_back(out->index);
  }
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfa::ProcessSimpleSignal(CF_NFA::CNfa &nfa, const CF_NFA::CNfaRaw::nfa_aux_state_t *out)
{
  nfa.m_dct_transitions[out->signal].push_back(out->index);
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfa::print_debug_info() const {
  if (m_dct_transitions.empty()) return;

  std::cout << m_number;
  std::cout << (m_is_final ? "F" : "");
  std::cout << " trans : " << std::endl;

  for (dct_transitions_t::const_iterator i = m_dct_transitions.cbegin(); i != m_dct_transitions.cend(); ++i) {
    std::cout << (char)i->first << " -> " ;

    for (std::vector<int>::const_iterator j = i->second.cbegin(); j != i->second.cend(); ++j) {
      std::cout << *j << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CNfa::print(const std::vector<CF_NFA::CNfa> &nfa_tbl)
{
  for (auto i = nfa_tbl.cbegin(); i != nfa_tbl.end(); ++i)
    i->print_debug_info();
}
////////////////////////////////////////////////////////////////////////////
