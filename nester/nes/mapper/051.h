#ifndef __NES_MAPPER_051_H
#define __NES_MAPPER_051_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 51
class NES_mapper51 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper51() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  void  Sync_Prg_Banks();
  uint8 bank, mode;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
