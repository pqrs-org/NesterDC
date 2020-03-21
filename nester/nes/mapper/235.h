#ifndef __NES_MAPPER_235_H
#define __NES_MAPPER_235_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 235
class NES_mapper235 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper235() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 dummy[0x2000];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
