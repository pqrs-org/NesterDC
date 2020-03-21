#ifndef __NES_MAPPER_018_H
#define __NES_MAPPER_018_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 18
class NES_mapper18 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper18() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;
  uint8 regs[11];
  uint8 irq_enabled;
  uint32 irq_latch;
  uint32 irq_counter;

  void SNSS_fixup();
private:
};
/////////////////////////////////////////////////////////////////////

#endif
