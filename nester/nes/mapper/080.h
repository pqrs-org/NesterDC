#ifndef __NES_MAPPER_080_H
#define __NES_MAPPER_080_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 80
class NES_mapper80 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper80() {}

  void  Reset();

  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
  uint8 patch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
