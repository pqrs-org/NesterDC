#ifndef __NES_MAPPER_180_H
#define __NES_MAPPER_180_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 180
class NES_mapper180 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper180() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
