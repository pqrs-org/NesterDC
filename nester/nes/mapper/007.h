#ifndef __NES_MAPPER_007_H
#define __NES_MAPPER_007_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 7
class NES_mapper7 : public NES_mapper
{
public:
  ~NES_mapper7() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);
protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
