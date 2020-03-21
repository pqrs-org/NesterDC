#ifndef __NES_MAPPER_101_H
#define __NES_MAPPER_101_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 101
class NES_mapper101 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper101() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
