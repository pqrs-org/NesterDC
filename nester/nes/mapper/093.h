#ifndef __NES_MAPPER_093_H
#define __NES_MAPPER_093_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 93
class NES_mapper93 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper93() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
