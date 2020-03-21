#ifndef __NES_MAPPER_009_H
#define __NES_MAPPER_009_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 9
class NES_mapper9 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper9() {}

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
