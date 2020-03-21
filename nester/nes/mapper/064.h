#ifndef __NES_MAPPER_064_H
#define __NES_MAPPER_064_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 64
class NES_mapper64 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper64() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 regs[3];
  uint8 irq_latch;
  uint8 irq_counter;
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
