#ifndef __NES_MAPPER_022_H
#define __NES_MAPPER_022_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 22
class NES_mapper22 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper22() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
