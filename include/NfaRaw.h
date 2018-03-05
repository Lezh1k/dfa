#ifndef CNFARAW_H
#define CNFARAW_H

#include "Commons.h"
#include "Re2Postfix.h"
#include <vector>
#include <iostream>

namespace CF_NFA {

  class CNfaRaw
  {
  public:

    ////////////////////////////////////////////////////////////////////////////
    /// aux structures and methods
    ////////////////////////////////////////////////////////////////////////////
    enum SpecialStates
    {
      SS_DOT   = -258,
      SS_SPLIT = -257, //get it from ktulhu
      SS_MATCH = -256,
      SS_START = -255,
      SS_MAX   = 258
    };

    typedef struct NfaAuxState {
      int32_t signal;
      NfaAuxState *out;
      NfaAuxState *out1;
      int32_t index;

      NfaAuxState(int32_t signal_, NfaAuxState *out_, NfaAuxState *out1_, int32_t index_) :
        signal(signal_),
        out(out_),
        out1(out1_),
        index(index_){}
    } nfa_aux_state_t;

    class NfaAuxStateComparator {
    public:
      bool operator ()(const NfaAuxState* arg1, const NfaAuxState* arg2) {
        return arg1->index < arg2->index;
      }
    };

  private:
    union AuxNfaStatesPtrList {
      AuxNfaStatesPtrList *next;
      nfa_aux_state_t *state;
    };

    typedef struct AuxFrag  {
      nfa_aux_state_t *start;
      AuxNfaStatesPtrList *out;
      AuxFrag(void) : start(NULL), out(NULL)
      {
      }
      AuxFrag(nfa_aux_state_t *_start, AuxNfaStatesPtrList *_out) : start(_start), out(_out){}
    } aux_frag_t;

    /* Create ptr_list from aux_state*/
    static AuxNfaStatesPtrList* aux_nfa_states_ptr_list(nfa_aux_state_t **outp) {
      AuxNfaStatesPtrList *l;
      l = (AuxNfaStatesPtrList *)outp;
      l->next = NULL;
      return l;
    }
    ////////////////////////////////////////////////////////////////////////////

    /* Patch the list of states at out to point to start. */
    static void patch(AuxNfaStatesPtrList *lst, nfa_aux_state_t *s) {
      AuxNfaStatesPtrList *next;
      for(;lst;lst=next){
        next=lst->next;
        lst->state=s;
      }
    }
    ////////////////////////////////////////////////////////////////////////////

    /* Join the two lists l1 and l2, returning the combination. */
    static AuxNfaStatesPtrList* append(AuxNfaStatesPtrList *l1, AuxNfaStatesPtrList *l2) {
      AuxNfaStatesPtrList *old_l1 = l1;
      while(l1->next) l1 = l1->next;
      l1->next = l2;
      return old_l1;
    }
    ////////////////////////////////////////////////////////////////////////////
    /// fields
    ////////////////////////////////////////////////////////////////////////////

    int m_last_error;    
    nfa_aux_state_t *m_match;
    std::vector<nfa_aux_state_t*> m_lst_states;

    static void print_signal(int signal);
  public:
    CNfaRaw();
    ~CNfaRaw(void);

    const std::vector<nfa_aux_state_t*>& LstStates(void) const {return m_lst_states;}

    ///
    /// \brief from_postfix
    /// \param nfa_raw - result structure.
    /// \param src_lst - vector with postfix regular expression
    /// \return result code. 0 if SUCCESS.
    ///
    static int from_postfix(CNfaRaw& nfa_raw, const std::vector<int> &src_lst);

//    void print_possible_ways(void) const;
    void print_nfa_raw(void) const;
  };

}
#endif // CNFARAW_H
