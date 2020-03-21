#ifndef __NES_MAPPER_100_H
#define __NES_MAPPER_100_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 100
class NES_mapper100 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper100() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8  regs[8];

  uint32 prg0,prg1,prg2,prg3;
  uint32 chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7;

  uint8 irq_enabled; // IRQs enabled
  uint8 irq_counter; // IRQ scanline counter, decreasing
  uint8 irq_latch;   // IRQ scanline counter latch

  void MMC3_set_CPU_banks()
  {
    set_CPU_banks(prg0,prg1,prg2,prg3);
  }

  void MMC3_set_PPU_banks()
  {
    if(num_1k_VROM_banks)
    {
      set_PPU_banks(chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7);
    }
  }

private:
};
/////////////////////////////////////////////////////////////////////

#endif
