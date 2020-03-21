#ifndef __NES_MAPPER_045_H
#define __NES_MAPPER_045_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 45
class NES_mapper45 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper45() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 patch;

  uint8  regs[7];
  uint32 p[4],prg0,prg1,prg2,prg3;
  uint32 c[8],chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7;

  uint8 irq_enabled;
  uint8 irq_counter;
  uint8 irq_latch;

  void MAP45_set_CPU_bank4(uint8 data);
  void MAP45_set_CPU_bank5(uint8 data);
  void MAP45_set_CPU_bank6(uint8 data);
  void MAP45_set_CPU_bank7(uint8 data);
  void MAP45_set_PPU_banks();

private:
};
/////////////////////////////////////////////////////////////////////

#endif
