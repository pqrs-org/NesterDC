#ifndef __NES_MAPPER_240_H
#define __NES_MAPPER_240_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 240
class NES_mapper240 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper240() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
