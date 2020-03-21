#ifndef __NES_MAPPER_019_H
#define __NES_MAPPER_019_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 19
class NES_mapper19 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper19() {}

  void  Reset();

  uint8 MemoryReadLow(uint32 addr);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;

  uint8 regs[3];
  uint8 irq_enabled;
  uint32 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
