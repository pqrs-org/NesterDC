#ifndef __NES_MAPPER_229_H
#define __NES_MAPPER_229_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 229
class NES_mapper229 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper229() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
