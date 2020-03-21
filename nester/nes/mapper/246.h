#ifndef __NES_MAPPER_246_H
#define __NES_MAPPER_246_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 246
class NES_mapper246 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper246() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
