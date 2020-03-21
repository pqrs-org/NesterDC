#ifndef __NES_MAPPER_071_H
#define __NES_MAPPER_071_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 71
class NES_mapper71 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper71() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
