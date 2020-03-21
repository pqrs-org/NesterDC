#ifndef __NES_MAPPER_230_H
#define __NES_MAPPER_230_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 230
class NES_mapper230 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper230() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 rom_switch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
