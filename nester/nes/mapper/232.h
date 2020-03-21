#ifndef __NES_MAPPER_232_H
#define __NES_MAPPER_232_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 232
class NES_mapper232 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper232() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[2];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
