#ifndef __NES_MAPPER_237_H
#define __NES_MAPPER_237_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 237
class NES_mapper237 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper237() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 wram[0x8000];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
