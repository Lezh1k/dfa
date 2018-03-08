#ifndef AHOKORASIC_H
#define AHOKORASIC_H

#include <stdint.h>
#include <vector>
#include <string>

namespace CF_NFA {
  class CAhoKorasic {
  private:
    struct vertex_t {
      static const int32_t NOT_INITIALIZED = -1;
      static const int32_t ALP_POWER = 256;

      int32_t next[ALP_POWER]; //
      int32_t go[ALP_POWER]; //

      int32_t parent; //parent index
      int32_t link; //suffix link
      int32_t isLeaf; //use int32_t for align

      uint8_t pCh; //char from parent to current state
      uint8_t padding[7];

      vertex_t();
      ~vertex_t();
    };

    //todo use some array .
    std::vector<vertex_t> m_dma;

    int32_t go(int32_t v, uint8_t ch);
    int32_t getLink(int32_t v);

  public:

    CAhoKorasic();
    ~CAhoKorasic();

    void addString(const std::vector<uint8_t> &str);
  };
}
#endif // AHOKORASIC_H
