#ifndef __NES_MAPPER_151_H
#define __NES_MAPPER_151_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 151
class NES_mapper151 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper151() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
