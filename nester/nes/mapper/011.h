#ifndef __NES_MAPPER_011_H
#define __NES_MAPPER_011_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 11
class NES_mapper11 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper11() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
