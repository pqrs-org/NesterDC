#ifndef __NES_MAPPER_043_H
#define __NES_MAPPER_043_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 43
class NES_mapper43 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper43() {}

  void  Reset();
  uint8 MemoryReadLow(uint32 addr);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;
  uint32 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
