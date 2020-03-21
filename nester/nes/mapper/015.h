#ifndef __NES_MAPPER_015_H
#define __NES_MAPPER_015_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 15
class NES_mapper15 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper15() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
