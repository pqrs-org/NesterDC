#ifndef __NES_MAPPER_026_H
#define __NES_MAPPER_026_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 26
class NES_mapper26 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper26() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;
  uint8 irq_counter;
  uint8 irq_latch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
