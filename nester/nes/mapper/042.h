#ifndef __NES_MAPPER_042_H
#define __NES_MAPPER_042_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 42
class NES_mapper42 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper42() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_counter;
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
