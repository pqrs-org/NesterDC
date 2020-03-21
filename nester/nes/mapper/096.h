#ifndef __NES_MAPPER_096_H
#define __NES_MAPPER_096_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 96
class NES_mapper96 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper96() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  PPU_Latch_Address(uint32 addr);

protected:
  void  sync_PPU_banks();
  uint8 vbank0,vbank1;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
