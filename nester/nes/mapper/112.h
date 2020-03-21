#ifndef __NES_MAPPER_112_H
#define __NES_MAPPER_112_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 112
class NES_mapper112 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper112() {}

  void  Reset();

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
  void MMC3_set_CPU_banks()
  {
    if(prg_swap())
    {
      set_CPU_banks(num_8k_ROM_banks-2,prg1,prg0,num_8k_ROM_banks-1);
    }
    else
    {
      set_CPU_banks(prg0,prg1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
    }
  }
  void MMC3_set_PPU_banks()
  {
    if(num_1k_VROM_banks)
    {
      if(chr_swap())
      {
        set_PPU_banks(chr4,chr5,chr6,chr7,chr01,chr01+1,chr23,chr23+1);
      }
      else
      {
        set_PPU_banks(chr01,chr01+1,chr23,chr23+1,chr4,chr5,chr6,chr7);
      }
    }
  }
  void SNSS_fixup();

private:
};
/////////////////////////////////////////////////////////////////////

#endif
