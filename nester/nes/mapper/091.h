#ifndef __NES_MAPPER_091_H
#define __NES_MAPPER_091_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 91
class NES_mapper91 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper91() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_counter;
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
