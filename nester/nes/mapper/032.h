#ifndef __NES_MAPPER_032_H
#define __NES_MAPPER_032_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 32
class NES_mapper32 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper32() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 patch;
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
