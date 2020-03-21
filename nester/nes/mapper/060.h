#ifndef __NES_MAPPER_060_H
#define __NES_MAPPER_060_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 60
class NES_mapper60 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper60() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
