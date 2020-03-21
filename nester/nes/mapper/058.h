#ifndef __NES_MAPPER_058_H
#define __NES_MAPPER_058_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 58
class NES_mapper58 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper58() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
