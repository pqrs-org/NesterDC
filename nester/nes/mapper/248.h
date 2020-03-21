#ifndef __NES_MAPPER_248_H
#define __NES_MAPPER_248_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 248
class NES_mapper248 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper248() {}

  void  Reset();

  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8  regs[8];

  uint32 prg0,prg1;
  uint32 chr01,chr23,chr4,chr5,chr6,chr7;

  uint32 chr_swap() { return regs[0] & 0x80; }
  uint32 prg_swap() { return regs[0] & 0x40; }

  uint8 irq_enabled; // IRQs enabled
  uint8 irq_counter; // IRQ scanline counter, decreasing
  uint8 irq_latch;   // IRQ scanline counter latch

  void MMC3_set_CPU_banks();
  void MMC3_set_PPU_banks();

  void SNSS_fixup(); // HACK HACK HACK HACK

private:
};
/////////////////////////////////////////////////////////////////////

#endif
