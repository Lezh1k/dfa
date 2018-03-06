#include <string.h>
#include "AhoKorasic.h"

//vertex!!
CAhoKorasic::vertex_t::vertex_t() :
  parent(NOT_INITIALIZED),
  pCh(NOT_INITIALIZED),
  link(NOT_INITIALIZED),
  isLeaf(0) {

  memset(next, 0, ALP_POWER*sizeof(int32_t));
  memset(go, 0, ALP_POWER*sizeof(int32_t));
}

CAhoKorasic::vertex_t::~vertex_t() {

}
//////////////////////////////////////////////////////////////////////////


CAhoKorasic::CAhoKorasic() {
  vertex_t root;
  m_dma.push_back(root);
}

CAhoKorasic::~CAhoKorasic() {
}
//////////////////////////////////////////////////////////////////////////

void CAhoKorasic::addString(const std::vector<uint8_t> &str) {
  int32_t v = 0;
  int32_t sz = (int32_t)m_dma.size();
  for (uint8_t c : str) {
    if (m_dma[v].next[c] == vertex_t::NOT_INITIALIZED) {
      vertex_t node;
      node.parent = v;
      node.pCh = c;
      m_dma[v].next[c] = sz++;
      m_dma.push_back(node);
    }
    v = m_dma[v].next[c];
  }
  m_dma[v].isLeaf = 1;
}
//////////////////////////////////////////////////////////////////////////

int32_t CAhoKorasic::go(int32_t v, uint8_t ch) {
  if (m_dma[v].go[ch] == vertex_t::NOT_INITIALIZED) { //if we don't have transition in go yet
    if (m_dma[v].next[ch] != vertex_t::NOT_INITIALIZED) { //check transition in next. if presented - just use it
      m_dma[v].go[ch] = m_dma[v].next[ch];
    } else { //
      m_dma[v].go[ch] = v == 0 ? 0 : go(getLink(v), ch); //if it's root use root. else use suffix link with current signal
    }
  }
  return m_dma[v].go[c];
}
//////////////////////////////////////////////////////////////////////////

int32_t CAhoKorasic::getLink(int32_t v) {
  if (m_dma[v].link == vertex_t::NOT_INITIALIZED) { //if we don't have link yet
    if (v == 0 || m_dma[v].p == 0) { //if it's root - return root
      m_dma[v].link = 0;
    } else {
      m_dma[v].link = go(getLink(m_dma[v].parent), m_dma[v].pch); //return transition from parrent's suffix link
    }
  }
  return m_dma[v].link;
}
//////////////////////////////////////////////////////////////////////////
