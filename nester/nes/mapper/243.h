#ifndef __NES_MAPPER_243_H
#define __NES_MAPPER_243_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 243
class NES_mapper243 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper243() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
  uint8 regs[4];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
