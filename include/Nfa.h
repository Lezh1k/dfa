#ifndef CNFA_H
#define CNFA_H

#include <vector>
#include <map>

#include <Commons.h>
#include <Re2Postfix.h>
#include <NfaRaw.h>

namespace CF_NFA {

  typedef std::map<int, std::vector<int> > dct_transitions_t; //keys is alphabet

  class CNfa
  {
  private:

    int m_number;
    bool m_is_final;
    dct_transitions_t m_dct_transitions;

    void print_debug_info() const ;

    ///
    /// \brief FillNfaTransitions
    /// \param nfa - current nfa state
    /// \param ss - we get transitions from this state
    ///
    static void FillNfaTransitions(CNfa& nfa, const CNfaRaw::nfa_aux_state_t* ss);
    static inline void ProcessDotSignal(CNfa& nfa, const CF_NFA::CNfaRaw::nfa_aux_state_t *out);
    static inline void ProcessSimpleSignal(CNfa& nfa, const CF_NFA::CNfaRaw::nfa_aux_state_t *out);

  public:
    CNfa();
    CNfa(const CNfa& nfa);
    ~CNfa(void);

    int number(void) const {return m_number;}
    bool is_final(void) const {return m_is_final;}
    const dct_transitions_t& dct_transitions(void) const {return m_dct_transitions;}

    ///
    /// \brief CreateFromRawNfa
    /// \param nfa_raw - raw nfa structure
    /// \return vector with filled nfa states
    ///
    static std::vector<CNfa> CreateFromRawNfa(const CNfaRaw& nfa_raw);
    static void print(const std::vector<CNfa>& nfa_tbl);
  };
}

#endif // CNFA_H
