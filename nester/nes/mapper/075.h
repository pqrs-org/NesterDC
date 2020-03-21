#ifndef __NES_MAPPER_075_H
#define __NES_MAPPER_075_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 75
class NES_mapper75 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper75() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[2];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
