#ifndef __NES_MAPPER_034_H
#define __NES_MAPPER_034_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 34
class NES_mapper34 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper34() {}

  void  Reset();

  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
