#ifndef __NES_MAPPER_231_H
#define __NES_MAPPER_231_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 231
class NES_mapper231 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper231() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
