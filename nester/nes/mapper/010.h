#ifndef __NES_MAPPER_010_H
#define __NES_MAPPER_010_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 10
class NES_mapper10 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper10() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

  void  PPU_Latch_FDFE(uint32 bank_base, uint32 bank);

protected:
  uint8 regs[6];
  uint8 latch_0000;
  uint8 latch_1000;

  void set_VROM_0000();
  void set_VROM_1000();

  void SNSS_fixup(); // HACK HACK HACK HACK

private:
};
/////////////////////////////////////////////////////////////////////

#endif
