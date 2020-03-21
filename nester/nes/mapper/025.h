#ifndef __NES_MAPPER_025_H
#define __NES_MAPPER_025_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 25
class NES_mapper25 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper25() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;
  uint8 regs[11];
  uint8 irq_enabled;
  uint8 irq_counter;
  uint8 irq_latch;

  void SNSS_fixup();
private:
};
/////////////////////////////////////////////////////////////////////

#endif
