#ifndef __NES_MAPPER_076_H
#define __NES_MAPPER_076_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 76
class NES_mapper76 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper76() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
