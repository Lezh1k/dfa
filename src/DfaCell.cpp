#include <iostream>

#include <algorithm>
#include <queue>
#include <stack>

#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#ifdef TIME_MEASUREMENTS
#include "IFunctor.h"
#include "FunctorWithoutResult.h"
#include "FunctorWithResult.h"
#include "TimeMeasurements.h"
#endif

#include "DfaCell.h"

CF_NFA::CDfaTable::CDfaTable(const std::vector<CDfa> &dfa_tbl, const alphabet_set_t &alp) :
  m_rows_mem((uint32_t)dfa_tbl.size()),
  m_cols((uint32_t)alp.size()),
  m_rows_real((uint32_t)dfa_tbl.size()),
  m_alphabet(NULL),
  m_used_in_patch_state(NULL),
  m_distances(NULL)
{
  m_table = new dfa_cell_t*[m_rows_mem];
  m_alphabet = new int16_t[m_cols];
  m_used_in_patch_state = new int8_t[m_rows_real];
  memset(m_used_in_patch_state, 0, m_rows_real);
  m_distances = new uint32_t[m_rows_real];
  memset(m_distances, 0, m_rows_real*sizeof(uint32_t));

  for (uint32_t i = 0; i < alp.size(); ++i)
    m_alphabet[i] = alp[i];

  for (uint32_t i = 0; i < m_rows_mem; ++i) {
    m_table[i] = new dfa_cell_t[m_cols];
    for (uint32_t j = 0; j < m_cols; ++j) {
      if (dfa_tbl[i].transitions()[j].empty()) continue;
      m_table[i][j].cell_val.state = dfa_tbl[i].transitions()[j][0];
    }
    m_table[i][0].cell_val.is_final = dfa_tbl[i].is_final();
  }
  //0 state IS NOT FINAL!!!!!!!!!!
  m_table[0]->cell_val.is_final = false;
}

CF_NFA::CDfaTable::~CDfaTable()
{
  if (m_alphabet) delete[] m_alphabet;
  if (m_table) {
    for (uint32_t i = 0; i < m_rows_mem; ++i)
      delete[] m_table[i];
    delete [] m_table;
  }
  if (m_used_in_patch_state) delete[] m_used_in_patch_state;
  if (m_distances) delete[] m_distances;
}
////////////////////////////////////////////////////////////////////////////

int CF_NFA::CDfaTable::compare_dfa_cells_row(int row1, int row2)
{
  return memcmp(m_table[row1], m_table[row2], m_cols*sizeof(dfa_cell_t));
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::remove_equivalent_states()
{
  register uint32_t i, j, i1, j1, nr = 0;
  for (i = 1; i < m_rows_mem-nr; ++i) {
    for (j = i+1; j < m_rows_mem-nr; ) {
      if (memcmp(m_table[i], m_table[j], m_cols*sizeof(dfa_cell_t))) {
        ++j; continue;
      }

      //change states == j to i
      for (i1 = 0; i1 < m_rows_mem-nr; ++i1) {
        for (j1 = 0; j1 < m_cols; ++j1) {
          if (m_table[i1][j1].cell_val.state == j)
            m_table[i1][j1].cell_val.state = i;
          if (m_table[i1][j1].cell_val.state == m_rows_mem-nr-1)
            m_table[i1][j1].cell_val.state = j;
        }
      }
      std::swap<dfa_cell_t*>(m_table[j], m_table[m_rows_mem-nr-1]);
      ++nr;
    }
  }
  m_rows_real -= nr;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::init_distances(void)
{
  std::queue<uint16_t> q;
  int8_t* used = new int8_t[m_rows_real];
  memset(used, 0, m_rows_real);
  q.push(0);
  used[0] = 1;
  m_distances[0] = 0;

  while (!q.empty()) {
    uint16_t v = q.front();
    q.pop();

    for (uint32_t i = 0; i < m_cols; ++i) {
      if (used[m_table[v][i].cell_val.state]) continue;
      used[m_table[v][i].cell_val.state] = 1;
      q.push(m_table[v][i].cell_val.state);
      m_distances[m_table[v][i].cell_val.state] =
          m_distances[v] + 1;
    }
  }
  delete[] used;
}
/////////////////////////////////////////////////////////////////////////

//todo use aho-korasic here.
void CF_NFA::CDfaTable::patch_way_to_state(uint16_t st_end)
{
  static int storage_size = 0;
  static std::vector<std::pair<uint16_t, uint16_t>> storage;

  storage.clear();
  memset(m_used_in_patch_state, 0, m_rows_real);
  storage.push_back({0, st_end});
  storage_size = 1;

  for(int last_index = 0; last_index != storage_size; ++last_index) {
    uint16_t src_v = storage[last_index].first;
    uint16_t dst_v = storage[last_index].second;
    if (m_table[dst_v]->cell_val.is_final)
      continue;

    for (uint32_t i = 0; i < m_cols; ++i) {
      uint16_t src_to = m_table[src_v][i].cell_val.state;
      uint16_t dst_to = m_table[dst_v][i].cell_val.state;

      if (src_to == 0) continue;
      if (m_used_in_patch_state[src_to]) continue;
      m_used_in_patch_state[src_to] = true;

      if (dst_to == 0) {
        if (src_v != 0) {
          m_table[dst_v][i].cell_val.state = m_table[src_v][i].cell_val.state;
          m_table[dst_v][i].cell_val.index_add = m_distances[st_end];
        }
        continue;
      }

      storage.push_back({src_to, dst_to});
      ++storage_size;
    } // for i < m_cols    
  } // for last_index != storage_size
}

void CF_NFA::CDfaTable::patch_for_loopbacks()
{
  //todo use aho-korasic here.
  //todo use aho-korasic here.
  //todo use aho-korasic here.
  //todo use aho-korasic here.
  //todo use aho-korasic here.!!!!!!
  init_distances();
  int8_t *used = new int8_t[m_rows_real];
  memset(used, 0, m_rows_real);
  for (uint32_t i = 0; i < m_cols; ++i) {
    if (m_table[0][i].cell_val.state == 0) continue;
    for (uint32_t j = 1; j < m_rows_real; ++j) {
      if (m_table[j][i].cell_val.state == 0) continue;
      if (used[j]) continue;
      patch_way_to_state(j);
      used[j] = 1;
    } //for j
  } //for i
  delete [] used;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::patch_for_all_ascii()
{
  dfa_cell_t** n_table = new dfa_cell_t*[m_rows_real];
  for (uint32_t i = 0; i < m_rows_real; ++i) {
    n_table[i] = new dfa_cell_t[CCommons::POSSIBLE_BYTE_VALUES_COUNT];
    for (uint32_t j = 0; j < m_cols; ++j) {
      n_table[i][m_alphabet[j]].w_val = m_table[i][j].w_val;
    }
    n_table[i][0].cell_val.is_final = m_table[i][0].cell_val.is_final;

    for (uint32_t j = 0; j < CCommons::POSSIBLE_BYTE_VALUES_COUNT; ++j) {
      n_table[i][j].cell_val.is_final = m_table[i][0].cell_val.is_final;
    }
  }  

  if (m_table) {
    for (uint32_t i = 0; i < m_rows_mem; ++i)
      delete[] m_table[i];
    delete [] m_table;
  }

  m_rows_mem = m_rows_real;
  m_cols = CCommons::POSSIBLE_BYTE_VALUES_COUNT;
  m_table = n_table;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::print() const
{
  printf("\nalphabet:\t");
  for (uint32_t i = 0; i < m_cols; ++i)
    printf("%c\t", (char)m_alphabet[i]);
  printf("\n");
  for (uint32_t i = 0; i < m_rows_real; ++i) {
    printf("\nstate : %d%s\t", (int)i , (m_table[i][0].cell_val.is_final ? "F" : ""));
    for (uint32_t j = 0; j < m_cols; ++j)
      printf("%d|%d\t", (int)m_table[i][j].cell_val.state, (int)m_table[i][j].cell_val.index_add);
  }
  printf("\n");
}
////////////////////////////////////////////////////////////////////////////

int8_t *CF_NFA::CDfaTable::serialize(uint32_t &size) const
{
  size = m_rows_real*m_cols*sizeof(dfa_cell_t);
  int8_t* buff = new int8_t[size];
  for (uint32_t i = 0; i < m_rows_real; ++i) {
    memcpy(buff+i*m_cols*sizeof(dfa_cell_t), m_table[i], m_cols*sizeof(dfa_cell_t));
  }
  return buff;
}
////////////////////////////////////////////////////////////////////////////

CF_NFA::CDfaTable *CF_NFA::CDfaTable::dfa_table_from_regexp(const char *re, int &res)
{
  CDfaTable* dfa = NULL;
  std::string pre = CRe2Postfix::regex2posix(re);
  uint8_t *buff = new uint8_t[pre.size()+2];
  memcpy(buff, pre.c_str(), pre.size() + 1);
  alphabet_set_t alp;

  std::vector<int> postfix = CRe2Postfix::regex2postfix(buff, res, alp);

  if (res != 0) return NULL;
  CNfaRaw raw_nfa;
  res = CNfaRaw::from_postfix(raw_nfa, postfix);
//  raw_nfa.print_nfa_raw();
  if (res == 0) {
    //todo change dot processing
    std::vector<CF_NFA::CNfa> nfa_table = CNfa::CreateFromRawNfa(raw_nfa);
//    CNfa::print(nfa_table);

    std::vector<CF_NFA::CDfa> dfa_table = CDfa::DfaTableFromNfa(nfa_table, alp);
//    CDfa::PrintDebugInfo(dfa_table, alp);
    //!!!!

    CDfa::RenameStatesInDfaTable(dfa_table);
//    CDfa::PrintDebugInfo(dfa_table, alp);

    dfa = new CDfaTable(dfa_table, alp);
//    dfa->print();

    dfa->patch_for_loopbacks();
//    dfa->print();

    dfa->remove_equivalent_states();
//    dfa->print();

    dfa->patch_for_all_ascii();
  }

  delete [] buff;
  return dfa;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int CF_NFA::CDfaTable::get_signal(char c) const
{
  return c;
  //todo change alphabet during patch_for_all_ascii
//  int index = CCommons::binary_search<int16_t>(m_alphabet, (int16_t)c, m_cols);
//  return index;
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::run_state_machine(const char* str, const char *re,
                                          void (*fp_found)(int,int,void*), void* callback_arg) const
{
  printf("\n***DEMO***\n");
  printf("Example string : %s\n", str);
  printf("Pattern string : %s\n", re);

  register const char* tmp = str;
  register int curr_st = 0;
  register int prev_st = 0;
  register int start = -1;
  register int match = -1;
  register int signal = -1;

  for (register int i = 0; tmp[i]; ++i) {

    signal = get_signal(tmp[i]);
    if (signal == -1) {
      if (match != -1) {
        fp_found(start, match, callback_arg);
      }
      start = match = -1;
      prev_st = curr_st = 0;
      continue;
    }

    prev_st = curr_st;
    curr_st = m_table[curr_st][signal].cell_val.state;

    if (prev_st == 0 && curr_st != 0)
      start = i;

    if (m_table[curr_st][0].cell_val.is_final) {
      match = i;
      continue;
    }

    if (prev_st != 0 && curr_st == 0) {
      if (match != -1) {
        fp_found(start, match, callback_arg);
        start = match = -1;
      }
    }

    if (curr_st == 0) {
      curr_st = m_table[curr_st][signal].cell_val.state;
      start = curr_st == 0 ? -1 : i;
      match = m_table[curr_st][0].cell_val.is_final ? i : -1;
      continue;
    }

    start += m_table[prev_st][signal].cell_val.index_add;
  } //for

  if (match == -1) return;
  fp_found(start, match, callback_arg);
}
////////////////////////////////////////////////////////////////////////////

void CF_NFA::CDfaTable::run_state_machine_file(const char *str)
{  
  register int curr_st = 0;
  register int prev_st = 0;
  register int start = -1;
  register int match = -1;

  register int count = 0;

  int inf = open(str, O_RDONLY | O_NOATIME);
  if (inf == -1) {
    printf("%s\n", str);
    perror("can't open input file");
    return;
  }

  register uint8_t *buffer = new uint8_t[16*512];
  int fd_in_size = 0;
  while((fd_in_size = read(inf, buffer, 16*512)) > 0) {
    for (register int i = 0; i < fd_in_size; ++i) {

      prev_st = curr_st;
      curr_st = m_table[curr_st][(int)buffer[i]].cell_val.state;

      if (prev_st == 0 && curr_st != 0)
        start = i;

      if (m_table[curr_st][0].cell_val.is_final) {
        match = i;
        continue;
      }

      if (prev_st != 0 && curr_st == 0) {
        if (match != -1) {
          start = match = -1;
          ++count;
        }
      }

      if (curr_st == 0) {
        curr_st = m_table[curr_st][(int)buffer[i]].cell_val.state;
        start = curr_st == 0 ? -1 : i;
        match = m_table[curr_st][0].cell_val.is_final ? i : -1;
        continue;
      }

      start += m_table[prev_st][(int)buffer[i]].cell_val.index_add;
    } //for
  }
  if (fd_in_size == -1)
    perror("err");
  if (match != -1) {++count;}
  printf("match count : %d\n", count);
  delete[] buffer;
  close(inf);
}
