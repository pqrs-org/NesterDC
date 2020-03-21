#ifndef __NES_MAPPER_066_H
#define __NES_MAPPER_066_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 66
class NES_mapper66 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper66() {}

  void  Reset();

  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
