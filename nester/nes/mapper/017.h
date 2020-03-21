#ifndef __NES_MAPPER_017_H
#define __NES_MAPPER_017_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 17
class NES_mapper17 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper17() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;
  uint32 irq_counter;
  uint32 irq_latch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
