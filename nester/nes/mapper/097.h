#ifndef __NES_MAPPER_097_H
#define __NES_MAPPER_097_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 97
class NES_mapper97 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper97() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
