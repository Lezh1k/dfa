#ifndef DFACELL_H
#define DFACELL_H

#include "Commons.h"
#include "Dfa.h"
#include <cstring>

namespace CF_NFA {

  class CDfaTable {

    /// obsolete. we used it with 16-bit digital design
    #ifdef DFA_CELL_16
    typedef struct DfaCell {
      union {
        struct {
          uint8_t   state     : 8;
          uint8_t   index_add : 7;
          bool      is_final  : 1;
        } cell_val;
        uint16_t w_val;
      };
      DfaCell(void) : w_val(0){}
    } dfa_cell_t ;
    #else
    /// Cell in dfa-table
    /// state - next state
    /// index_add - this value should be added to start index
    /// of begining of match if we have repeated sequence.
    /// for example in "cadcadcal" if we have input string like
    /// this : "cadcadcadcal" we should find start as 3. not as 0.
    /// is_final : determines state is final
    typedef struct DfaCell {
      union {
        struct {
          uint16_t   state     : 16;
          uint16_t   index_add : 15;
          bool      is_final  : 1;
        } cell_val;
        uint32_t w_val;
      };
      DfaCell(void) : w_val(0){}
    } dfa_cell_t ;
    #endif

  private:

//    static const uint16_t INF_ST = 0xffff;
    enum {INF_ST = 0xffff};

    uint32_t m_rows_mem;
    uint32_t m_cols;
    uint32_t m_rows_real;

    dfa_cell_t **m_table;
    int16_t *m_alphabet;

    int8_t *m_used_in_patch_state;
    uint32_t *m_distances; //from zero state

    CDfaTable(void);
    CDfaTable(const CDfaTable&);
    void operator=(const CDfaTable&);
    //copy, default constructor and assigment are prohibited

    void init_distances(void);

    ///
    /// \brief find_way_to_state
    /// \param st_end - we try to find way from zero state to this state.
    /// \param N - result stack size.
    /// \return stack that represents way from st_end to zero.
    ///
    void patch_way_to_state(uint16_t st_end);

    ///
    /// \brief compare_dfa_cells_row
    /// \param row1 - index in dfa-table
    /// \param row2 - index in dfa-table
    /// \return 0 if row1 == row2. another value if row1 != row2
    ///
    inline int compare_dfa_cells_row(int row1, int row2);

    ///
    /// \brief get_signal now not used.
    /// \param c - input character
    /// \return - return column index in dfa-table
    ///
    int get_signal(char c) const;

    CDfaTable(const std::vector<CF_NFA::CDfa> &dfa_tbl, const alphabet_set_t &alp);
  public:

    ~CDfaTable(void);

    ///
    /// \brief remove_equivalent_states
    ///
    void remove_equivalent_states(void);

    ///
    /// \brief patch_for_loopbacks
    /// This function adds value of index_add to each cell
    ///
    void patch_for_loopbacks(void);

    ///
    /// \brief patch_for_all_ascii
    /// This function creates new dfa-table from old dfa-table
    /// and adds all ASCII values to new dfa-table.
    ///
    void patch_for_all_ascii(void);

    ///
    /// \brief serialize
    /// \param size - result size
    /// \return serialized into int8_t array dfa-table
    ///
    int8_t* serialize(uint32_t &size) const;

    ///
    /// \brief print
    ///
    void print(void) const;

    ///
    /// \brief dfa_table_from_regexp
    /// \param re - input regular expression
    /// \param res - result code
    /// \return dfa-table or NULL
    ///
    static CDfaTable *dfa_table_from_regexp(const char* re, int &res);

    ///
    /// \brief run_state_machine
    /// \param str - input string
    /// \param re - regular expression
    /// \param void (*fp_found)(int, int, void*) - callback function
    /// It calls when match is found
    /// \param callback_arg - argument in fp_found callback function
    ///
    void run_state_machine(const char* str, const char *re,
                           void (*fp_found)(int,int,void*), void* callback_arg) const;

    ///
    /// \brief run_state_machine_file
    /// \param str - file name.
    ///
    void run_state_machine_file(const char* str);
  };
}

#endif // DFACELL_H
