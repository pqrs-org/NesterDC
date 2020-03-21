#ifndef __NES_MAPPER_234_H
#define __NES_MAPPER_234_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 234
class NES_mapper234 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper234() {}

  void  Reset();
  void  MemoryReadSaveRAM(uint32 addr);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  void Sync();
  uint8 regs[3];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
