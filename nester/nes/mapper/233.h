#ifndef __NES_MAPPER_233_H
#define __NES_MAPPER_233_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 233
class NES_mapper233 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper233() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
