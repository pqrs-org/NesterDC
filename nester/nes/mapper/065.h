#ifndef __NES_MAPPER_065_H
#define __NES_MAPPER_065_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 65
class NES_mapper65 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper65() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;

  uint8 irq_enabled;
  uint32 irq_counter;
  uint32 irq_latch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
