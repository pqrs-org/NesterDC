#ifndef __NES_MAPPER_033_H
#define __NES_MAPPER_033_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 33
class NES_mapper33 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper33() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;
  uint8 irq_enabled;
  uint8 irq_counter;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
