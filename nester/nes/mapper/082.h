#ifndef __NES_MAPPER_082_H
#define __NES_MAPPER_082_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 82
class NES_mapper82 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper82() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
