#ifndef __NES_MAPPER_049_H
#define __NES_MAPPER_049_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 49
class NES_mapper49 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper49() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8  regs[3];
  uint32 prg0,prg1;
  uint32 chr01,chr23,chr4,chr5,chr6,chr7;

  uint8 irq_enabled;
  uint8 irq_counter;
  uint8 irq_latch;

  void MMC3_set_CPU_banks();
  void MMC3_set_PPU_banks();

  void SNSS_fixup();

private:
};
/////////////////////////////////////////////////////////////////////

#endif
