#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "024.h"

/////////////////////////////////////////////////////////////////////
// Mapper 24
void NES_mapper24::Reset()
{
  // Init ExSound
  parent_NES->apu->SelectExSound(NES_APU_EXSOUND_VRC6);

  // set CPU bank pointers
  set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

  // set PPU bank pointers
  if(num_1k_VROM_banks)
  {
    set_PPU_banks(0,1,2,3,4,5,6,7);
  }

  irq_enabled_l = 0;
  irq_enabled_h = 0;
  irq_counter = 0;
  irq_latch = 0;
}

void
NES_mapper24::MemoryWrite(uint32 addr, uint8 data)
{
  switch (addr & 0xF003)
  {
    case 0x8000:
      set_CPU_bank4(data*2+0);
      set_CPU_bank5(data*2+1);
      break;
      
    case 0xB003:
    {
      data &= 0x0C;
      if(data == 0x00)
        set_mirroring(NES_PPU::MIRROR_VERT);
      else if(data == 0x04)
        set_mirroring(NES_PPU::MIRROR_HORIZ);
      else if(data == 0x08)
        set_mirroring(0,0,0,0);
      else if(data == 0x0C)
        set_mirroring(1,1,1,1);
    }
    break;
    
    case 0xC000:
      set_CPU_bank6(data);
      break;
      
    case 0xd000:
    case 0xd001:
    case 0xd002:
    case 0xd003:
      set_PPU_bank_nth((addr & 0xf003) - 0xd000, data);
      break;
      
    case 0xe000:
    case 0xe001:
    case 0xe002:
    case 0xe003:
      set_PPU_bank_nth((addr & 0xf003) - 0xe000 + 4, data);
      break;
      
    case 0xF000:
      irq_latch = data;
      break;
      
    case 0xF001:
      irq_enabled_l = data & 0x1;
      irq_enabled_h = data & 0x2;
      if (irq_enabled_h)
        irq_counter = irq_latch;
      break;
      
    case 0xF002:
      irq_enabled_h = irq_enabled_l;
      break;
  }
  
  parent_NES->apu->ExWrite(addr, data);
}

void NES_mapper24::HSync(uint32 scanline)
{
  if (irq_enabled_h)
  {
    if(irq_counter == 0xff)
    {
      parent_NES->cpu->DoIRQ();
      irq_counter = irq_latch;
    }
    else
    {
      irq_counter++;
    }
  }
}
/////////////////////////////////////////////////////////////////////

