#ifndef __NES_MAPPER_122_H
#define __NES_MAPPER_122_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 122
class NES_mapper122 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper122() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
