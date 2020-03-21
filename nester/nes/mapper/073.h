#ifndef __NES_MAPPER_073_H
#define __NES_MAPPER_073_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 73
class NES_mapper73 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper73() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;
  uint32 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
