#ifndef __NES_MAPPER_003_H
#define __NES_MAPPER_003_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 3
class NES_mapper3 : public NES_mapper
{
public:
  ~NES_mapper3() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
