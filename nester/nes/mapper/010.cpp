#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "010.h"

/////////////////////////////////////////////////////////////////////
// Mapper 10
void NES_mapper10::Reset()
{
  // set CPU bank pointers
  set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

  for(uint32 i = 0; i < sizeof(regs)/sizeof(regs[0]); i++)
    regs[i] = 0;

  regs[2] = 4;

  latch_0000 = 1;
  latch_1000 = 1;

  set_VROM_0000();
  set_VROM_1000();
}

void NES_mapper10::PPU_Latch_FDFE(uint32 bank_base, uint32 bank)
{
  if (bank_base)
  {
    latch_1000 = bank;
    set_VROM_1000();
  }
  else
  {
    latch_0000 = bank;
    set_VROM_0000();
  }
}

void NES_mapper10::set_VROM_0000()
{
  int bank_num = regs[latch_0000 + 1] << 2;

  set_PPU_bank0(bank_num+0);
  set_PPU_bank1(bank_num+1);
  set_PPU_bank2(bank_num+2);
  set_PPU_bank3(bank_num+3);
}

void NES_mapper10::set_VROM_1000()
{
  int bank_num = regs[latch_1000 + 3] << 2;

  set_PPU_bank4(bank_num+0);
  set_PPU_bank5(bank_num+1);
  set_PPU_bank6(bank_num+2);
  set_PPU_bank7(bank_num+3);
}

void NES_mapper10::MemoryWrite(uint32 addr, uint8 data)
{
  switch(addr & 0xF000)
  {
    case 0xA000:
      {
        regs[0] = data;

        // 16K ROM bank at $8000
        uint8 bank_num = regs[0];
        set_CPU_bank4(bank_num*2);
        set_CPU_bank5(bank_num*2+1);
      }
      break;

    case 0xB000:
      {
        // B000-BFFF: select 4k VROM for (0000) $FD latch
        regs[1] = data;
        set_VROM_0000();
      }
      break;

    case 0xC000:
      {
        // C000-CFFF: select 4k VROM for (0000) $FE latch
        regs[2] = data;
        set_VROM_0000();
      }
      break;

    case 0xD000:
      {
        // D000-DFFF: select 4k VROM for (1000) $FD latch
        regs[3] = data;
        set_VROM_1000();
      }
      break;

    case 0xE000:
      {
        // E000-EFFF: select 4k VROM for (1000) $FE latch
        regs[4] = data;
        set_VROM_1000();
      }
      break;

    case 0xF000:
      {
        regs[5] = data;

        if(regs[5] & 0x01)
        {
          set_mirroring(NES_PPU::MIRROR_HORIZ);
        }
        else
        {
          set_mirroring(NES_PPU::MIRROR_VERT);
        }
      }
      break;
  }
}

void NES_mapper10::SNSS_fixup()
{
  set_VROM_0000();
  set_VROM_1000();
}
/////////////////////////////////////////////////////////////////////

