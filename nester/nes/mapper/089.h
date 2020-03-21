#ifndef __NES_MAPPER_089_H
#define __NES_MAPPER_089_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 89
class NES_mapper89 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper89() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
