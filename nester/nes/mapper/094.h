#ifndef __NES_MAPPER_094_H
#define __NES_MAPPER_094_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 94
class NES_mapper94 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper94() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
