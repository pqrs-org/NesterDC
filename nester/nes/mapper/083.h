#ifndef __NES_MAPPER_083_H
#define __NES_MAPPER_083_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 83
class NES_mapper83 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper83() {}

  void  Reset();
  uint8 MemoryReadLow(uint32 addr);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 regs[3];
  uint32 irq_counter;
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
