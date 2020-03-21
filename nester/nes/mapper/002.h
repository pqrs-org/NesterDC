#ifndef __NES_MAPPER_002_H
#define __NES_MAPPER_002_H

#include "nes_mapper.h"
#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 2
class NES_mapper2 : public NES_mapper
{
public:
  ~NES_mapper2() {}
  
  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
