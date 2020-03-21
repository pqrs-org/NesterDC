#ifndef __NES_MAPPER_087_H
#define __NES_MAPPER_087_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 87
class NES_mapper87 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper87() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
