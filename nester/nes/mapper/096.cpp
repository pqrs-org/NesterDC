#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "096.h"

/////////////////////////////////////////////////////////////////////
// Mapper 96
void NES_mapper96::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,1,2,3);

  // set PPU bank pointers
  vbank0 = vbank1 = 0;
  sync_PPU_banks();
  
  set_mirroring(0,0,0,0);
}

void NES_mapper96::PPU_Latch_Address(uint32 addr)
{
  if((addr & 0xF000) == 0x2000)
  {
    vbank1 = (addr & 0x0300) >> 8;
    sync_PPU_banks();
  }
}

void NES_mapper96::MemoryWrite(uint32 addr, uint8 data)
{
  set_CPU_bank4((data & 0x03) * 4 + 0);
  set_CPU_bank5((data & 0x03) * 4 + 1);
  set_CPU_bank6((data & 0x03) * 4 + 2);
  set_CPU_bank7((data & 0x03) * 4 + 3);

  vbank0 = (data & 0x04) >> 2;
  sync_PPU_banks();
}

void NES_mapper96::sync_PPU_banks()
{
  set_VRAM_bank0(0, vbank0 * 16 + vbank1 * 4 + 0);
  set_VRAM_bank0(1, vbank0 * 16 + vbank1 * 4 + 1);
  set_VRAM_bank0(2, vbank0 * 16 + vbank1 * 4 + 2);
  set_VRAM_bank0(3, vbank0 * 16 + vbank1 * 4 + 3);
  set_VRAM_bank0(4, vbank0 * 16 + 12);
  set_VRAM_bank0(5, vbank0 * 16 + 13);
  set_VRAM_bank0(6, vbank0 * 16 + 14);
  set_VRAM_bank0(7, vbank0 * 16 + 15);
}
/////////////////////////////////////////////////////////////////////

