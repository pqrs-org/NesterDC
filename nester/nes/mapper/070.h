#ifndef __NES_MAPPER_070_H
#define __NES_MAPPER_070_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 70
class NES_mapper70 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper70() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 patch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
