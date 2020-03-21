#ifndef __NES_MAPPER_072_H
#define __NES_MAPPER_072_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 72
class NES_mapper72 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper72() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
