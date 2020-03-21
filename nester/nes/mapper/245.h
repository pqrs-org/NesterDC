#ifndef __NES_MAPPER_245_H
#define __NES_MAPPER_245_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 245
class NES_mapper245 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper245() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
