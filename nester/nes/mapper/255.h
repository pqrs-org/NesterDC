#ifndef __NES_MAPPER_255_H
#define __NES_MAPPER_255_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 255
class NES_mapper255 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper255() {}

  void  Reset();
  uint8 MemoryReadLow(uint32);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[4];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
