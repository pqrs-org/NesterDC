#ifndef __NES_MAPPER_008_H
#define __NES_MAPPER_008_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 8
class NES_mapper8 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper8() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
