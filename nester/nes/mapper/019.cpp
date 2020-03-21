#include "types.h"
#include "nes.h"
#include "nes_mapper.h"
#include "019.h"

void
NES_mapper19::Reset()
{
  patch = 0;
  
  if(parent_NES->crc32() == 0x761ccfb5 || // Digital Devil Story - Megami Tensei 2
     parent_NES->crc32() == 0x1294ab5a)   // Digital Devil Story - Megami Tensei 2
  {
    patch = 1;
  }
  
  if(parent_NES->crc32() == 0x96533999 || // Dokuganryuu Masamune
     parent_NES->crc32() == 0x429fd177 || // Famista '90
     parent_NES->crc32() == 0xdd454208 || // Hydlide 3 - Yami Kara no Houmonsha (J).nes
     parent_NES->crc32() == 0xb1b9e187 || // Kaijuu Monogatari
     parent_NES->crc32() == 0xaf15338f)   // Mindseeker
  {
    patch = 2;
  }
  
  // Init ExSound
  parent_NES->apu->SelectExSound(NES_APU_EXSOUND_N106);
  if (parent_NES->crc32() == 0x14942c06 || // Wagyan Land 3 (j)
      parent_NES->crc32() == 0x02738c68) // Youkai Doutyu ki (j)
  {
    parent_NES->apu->SelectExSound(NES_APU_EXSOUND_NONE);
  }
  
  // set CPU bank pointers
  set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
  
  // set PPU bank pointers
  if(num_1k_VROM_banks >= 8)
  {
    set_PPU_bank0(num_1k_VROM_banks-8);
    set_PPU_bank1(num_1k_VROM_banks-7);
    set_PPU_bank2(num_1k_VROM_banks-6);
    set_PPU_bank3(num_1k_VROM_banks-5);
    set_PPU_bank4(num_1k_VROM_banks-4);
    set_PPU_bank5(num_1k_VROM_banks-3);
    set_PPU_bank6(num_1k_VROM_banks-2);
    set_PPU_bank7(num_1k_VROM_banks-1);
  }

  regs[0] = 0;
  regs[1] = 0;
  regs[2] = 0;
}

uint8 NES_mapper19::MemoryReadLow(uint32 addr)
{
  if(addr == 0x4800)
  {
    if(patch == 2)
    {
      uint8 retval =  parent_NES->ReadSaveRAM(regs[2] & 0x7F);
      if(regs[2] & 0x80) regs[2] = ((regs[2] & 0x7F)+1) | 0x80;
      return retval;
    }
    else
    {
      return parent_NES->apu->ExRead(addr);
    }
  }
  else if((addr & 0xF800) == 0x5000) // addr $5000-$57FF
  {
    return (uint8)(irq_counter & 0xff);
  }
  else if ((addr & 0xF800) == 0x5800) // addr $5800-$5FFF
  {
    return (uint8)((irq_counter >> 8) & 0x7f);
  }
  else
  {
    return (uint8)(addr >> 8);
  }
}

void
NES_mapper19::MemoryWriteLow(uint32 addr, uint8 data)
{
  switch(addr & 0xF800)
  {
    case 0x4800:
    {
      if(addr == 0x4800)
      {
        if(patch == 2)
        {
          parent_NES->WriteSaveRAM(regs[2] & 0x7F, data);
          if(regs[2] & 0x80) regs[2] = ((regs[2] & 0x7F)+1) | 0x80;
        }
        else
        {
          parent_NES->apu->ExWrite(addr, data);
        }
      }
    }
    break;
    
    case 0x5000: // addr $5000-$57FF
    {
      irq_counter = (irq_counter & 0xFF00) | data;
    }
    break;
    
    case 0x5800: // addr $5000-$57FF
    {
      irq_counter = (irq_counter & 0x00FF) | ((uint32)(data & 0x7F) << 8);
      irq_enabled = (data & 0x80) >> 7;
      if(patch)
      {
        irq_counter ++;
      }
    }
    break;
  }
}

void
NES_mapper19::MemoryWrite(uint32 addr, uint8 data)
{
  uint32 addr_bank = (addr >> 11) & 0x1f;
  
  if (addr_bank < 0x10) 
  {
    return;
  }
  else if (addr_bank < 0x18) 
  {
    if (data < 0xe0 | regs[0] == 1)
      set_PPU_bank_nth(addr_bank - 0x10, data);
    else
      set_VRAM_bank0(addr_bank - 0x10, addr_bank - 0x10);
  }
  else if (addr_bank < 0x1c)
  {
    if(data <= 0xdf)
      set_PPU_bank_nth(addr_bank - 0x10, data);
    else
      set_VRAM_bank8(addr_bank - 0x10, data & 0x01);
  }
  else 
  {
    switch (addr_bank)
    {
      case 0x1c: /* addr $e000-$e7ff */
        set_CPU_bank4(data & 0x3F);
        break;
        
      case 0x1d: /* addr $e800-$efff */
        set_CPU_bank5(data & 0x3F);
        regs[0] = (data >> 6) & 0x1;
        regs[1] = (data >> 7) & 0x1;
        break;
        
      case 0x1e: /* addr $f000-$f7ff */
        set_CPU_bank6(data & 0x3F);
        break;
        
      case 0x1f:   
        if(addr == 0xF800)
        {
          if(patch == 2)
            regs[2] = data;
          else
            parent_NES->apu->ExWrite(addr, data);
        }
    }
  }
}


void
NES_mapper19::HSync(uint32 scanline)
{
  if(irq_enabled)
  {
    if(irq_counter >= (uint32)(0x7FFF - ((patch == 1) ? 112 : 113)))
    {
      irq_counter = 0x7FFF;
      parent_NES->cpu->DoIRQ();
    }
    else
    {
      irq_counter += ((patch == 1) ? 112 : 113);
    }
  }
}

