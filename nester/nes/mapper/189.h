#ifndef __NES_MAPPER_189_H
#define __NES_MAPPER_189_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 189
class NES_mapper189 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper189() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 regs[1];
  uint8 irq_counter;
  uint8 irq_latch;
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
