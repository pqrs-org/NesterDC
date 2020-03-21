#ifndef __NES_MAPPER_183_H
#define __NES_MAPPER_183_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 183
class NES_mapper183 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper183() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 regs[8];
  uint8 irq_enabled;
  uint32 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
