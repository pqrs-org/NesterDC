#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "013.h"

/////////////////////////////////////////////////////////////////////
// Mapper 13
void NES_mapper13::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,1,2,3);

  // set PPU bank pointers
  set_VRAM_bank0(0, 0);
  set_VRAM_bank0(1, 1);
  set_VRAM_bank0(2, 2);
  set_VRAM_bank0(3, 3);
  set_VRAM_bank0(4, 0);
  set_VRAM_bank0(5, 1);
  set_VRAM_bank0(6, 2);
  set_VRAM_bank0(7, 3);

  prg_bank = chr_bank = 0;
}

void NES_mapper13::MemoryWrite(uint32 addr, uint8 data)
{
  prg_bank = (data & 0x30) >> 4;
  chr_bank = data & 0x03;

  set_CPU_bank4(prg_bank*4+0);
  set_CPU_bank5(prg_bank*4+1);
  set_CPU_bank6(prg_bank*4+2);
  set_CPU_bank7(prg_bank*4+3);

  set_VRAM_bank0(4, chr_bank * 4 + 0);
  set_VRAM_bank0(5, chr_bank * 4 + 1);
  set_VRAM_bank0(6, chr_bank * 4 + 2);
  set_VRAM_bank0(7, chr_bank * 4 + 3);
}

void NES_mapper13::SNSS_fixup()
{
  set_VRAM_bank0(4, chr_bank * 4 + 0);
  set_VRAM_bank0(5, chr_bank * 4 + 1);
  set_VRAM_bank0(6, chr_bank * 4 + 2);
  set_VRAM_bank0(7, chr_bank * 4 + 3);
}
/////////////////////////////////////////////////////////////////////

