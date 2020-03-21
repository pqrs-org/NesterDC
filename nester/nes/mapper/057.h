#ifndef __NES_MAPPER_057_H
#define __NES_MAPPER_057_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 57
class NES_mapper57 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper57() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
