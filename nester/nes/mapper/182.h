#ifndef __NES_MAPPER_182_H
#define __NES_MAPPER_182_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 182
class NES_mapper182 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper182() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 regs[1];
  uint8 irq_enabled;
  uint8 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
