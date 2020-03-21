#ifndef __NES_MAPPER_226_H
#define __NES_MAPPER_226_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 226
class NES_mapper226 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper226() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 regs[2];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
