#ifndef __NES_MAPPER_086_H
#define __NES_MAPPER_086_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 86
class NES_mapper86 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper86() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
