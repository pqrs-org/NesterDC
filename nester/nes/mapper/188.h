#ifndef __NES_MAPPER_188_H
#define __NES_MAPPER_188_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 188
class NES_mapper188 : public NES_mapper
{
public:
  ~NES_mapper188() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 dummy[0x2000];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
