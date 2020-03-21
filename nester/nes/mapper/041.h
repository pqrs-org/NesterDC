#ifndef __NES_MAPPER_041_H
#define __NES_MAPPER_041_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 41
class NES_mapper41 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper41() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
