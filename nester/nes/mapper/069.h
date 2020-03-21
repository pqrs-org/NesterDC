#ifndef __NES_MAPPER_069_H
#define __NES_MAPPER_069_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 69
class NES_mapper69 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper69() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;
  uint8 regs[1];
  uint8 irq_enabled;
  uint32 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
