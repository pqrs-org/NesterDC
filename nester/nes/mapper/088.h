#ifndef __NES_MAPPER_088_H
#define __NES_MAPPER_088_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 88
class NES_mapper88 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper88() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8  regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
