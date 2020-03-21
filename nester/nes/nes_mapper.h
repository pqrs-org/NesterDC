/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#ifndef _NES_MAPPER_H_
#define _NES_MAPPER_H_

#include "types.h"
#include "nes_rom.h"
#include "debug.h"

class NES;  // class prototypes
class NES_mapper;
class NES_SNSS;

/////////////////////////////////////////////////////////////////////
// mapper factory
NES_mapper* GetMapper(int mapper_num);


/////////////////////////////////////////////////////////////////////
// Mapper virtual base class
class NES_mapper
{
public:
  virtual ~NES_mapper() {};
  virtual bool initialize (NES *parent, NES_PPU *ppu);
  
  virtual void  Reset() = 0;
  
  virtual uint8 MemoryReadLow(uint32 addr) {return (uint8)(addr >> 8);}
  virtual void  WriteHighRegs(uint32 addr, uint8 data) {}
  virtual void  MemoryWrite(uint32 addr, uint8 data) {}
  virtual void  MemoryWriteLow(uint32 addr, uint8 data) {}
  virtual void  MemoryWriteSaveRAM(uint32 addr, uint8 data) {}
  
  virtual void  MemoryReadSaveRAM(uint32 addr) {}

  virtual void  HSync(uint32 scanline) {}
  virtual void  VSync()  {}

  // for mmc2 & mmc5 & Oekakidds
  virtual void  PPU_Latch_FDFE(uint32 bank_base, uint32 bank) {}
  virtual uint8 PPU_Latch_RenderScreen(uint8 mode, uint32 addr) {return 0;}
  virtual void  PPU_Latch_Address(uint32 addr) {}

  // for disk system (#20)
  virtual uint8 GetDiskSideNum()                   {return 0;}
  virtual uint8 GetDiskSide()                      {return 0;}
  virtual void  SetDiskSide(uint8 side)            {}
  virtual uint8 GetDiskData(uint32 pt)             {return 0;}
  virtual void  SetDiskData(uint32 pt, uint8 data) {}
  virtual uint8 DiskAccessed()                     {return 0;}

  // for Datach Barcode Battler
  virtual void SetBarcodeValue(uint32 value_low, uint32 value_high) {}

protected:
  NES* parent_NES;
  NES_PPU *attached_ppu;
  
  uint32 num_16k_ROM_banks;
  uint32 num_8k_ROM_banks;
  uint32 num_1k_VROM_banks;

  uint8* ROM_banks;
  uint8* VROM_banks;
  
  // for ROM & VROM over 256KB
  uint32 ROM_mask;
  uint32 VROM_mask;
  
  void set_CPU_banks(uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  // for mapper 40
  void set_CPU_banks(uint32 bank3_num,
                     uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  
  /* force inline */
  inline void set_CPU_bank3(uint32 bank_num) 
  {
    (nes6502_get_current_context())->mem_page[3] = ROM_banks + ((bank_num & ROM_mask) << 13); // * 0x2000
    nes6502_setcontext();
  }
  
  inline void set_CPU_bank4(uint32 bank_num) 
  {
    (nes6502_get_current_context())->mem_page[4] = ROM_banks + ((bank_num & ROM_mask) << 13);
    nes6502_setcontext();
  }
  
  inline void set_CPU_bank5(uint32 bank_num)
  {
    (nes6502_get_current_context())->mem_page[5] = ROM_banks + ((bank_num & ROM_mask) << 13);
    nes6502_setcontext();
  }
  
  inline void set_CPU_bank6(uint32 bank_num)
  {
    (nes6502_get_current_context())->mem_page[6] = ROM_banks + ((bank_num & ROM_mask) << 13);
    nes6502_setcontext();
  }
  
  inline void set_CPU_bank7(uint32 bank_num)
  {
    (nes6502_get_current_context())->mem_page[7] = ROM_banks + ((bank_num & ROM_mask) << 13);
    nes6502_setcontext();
  }
  
  void set_PPU_banks(uint32 bank0_num, uint32 bank1_num,
                     uint32 bank2_num, uint32 bank3_num,
                     uint32 bank4_num, uint32 bank5_num,
                     uint32 bank6_num, uint32 bank7_num);
  
  /* force inline */
  inline void set_PPU_bank_nth(uint32 nth, uint32 bank_num)
  {
    attached_ppu->PPU_VRAM_banks[nth] = VROM_banks + ((bank_num & VROM_mask) << 10);
  }
  
  inline void set_PPU_bank0(uint32 bank_num) 
  {
    set_PPU_bank_nth (0, bank_num);
  }
  
  inline void set_PPU_bank1(uint32 bank_num) 
  {
    set_PPU_bank_nth (1, bank_num);
  }
  
  inline void set_PPU_bank2(uint32 bank_num)
  {
    set_PPU_bank_nth (2, bank_num);
  }
  
  inline void set_PPU_bank3(uint32 bank_num)
  {
    set_PPU_bank_nth (3, bank_num);
  }
  
  inline void set_PPU_bank4(uint32 bank_num)
  {
    set_PPU_bank_nth (4, bank_num);
  }
  
  inline void set_PPU_bank5(uint32 bank_num)
  {
    set_PPU_bank_nth (5, bank_num);
  }
  
  inline void set_PPU_bank6(uint32 bank_num)
  {
    set_PPU_bank_nth (6, bank_num);
  }
  
  inline void set_PPU_bank7(uint32 bank_num)
  {
    set_PPU_bank_nth (7, bank_num);
  }
  
  // for mapper 19,68,90
  inline void set_PPU_bank8(uint32 bank_num)
  {
    set_PPU_bank_nth (8, bank_num);
  }
  
  inline void set_PPU_bank9(uint32 bank_num)
  {
    set_PPU_bank_nth (9, bank_num);
  }
  
  inline void set_PPU_bank10(uint32 bank_num)
  {
    set_PPU_bank_nth (10, bank_num);
  }
  
  inline void set_PPU_bank11(uint32 bank_num)
  {
    set_PPU_bank_nth (11, bank_num);
  }
  
  // For mapper 1,4,5,6,13,19,80,85,96,119
  inline void set_VRAM_bank0 (uint8 bank, uint32 bank_num)
  {
    attached_ppu->PPU_VRAM_banks[bank] = attached_ppu->get_patt() + ((bank_num & 0x0f) << 10);
  }
  
  inline void set_VRAM_bank8 (uint8 bank, uint32 bank_num)
  {
    attached_ppu->PPU_VRAM_banks[bank] = attached_ppu->get_namt() + ((bank_num & 0x03) << 10);
  }
  
  inline void set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3) 
  {
    attached_ppu->set_mirroring(nt0,nt1,nt2,nt3);
  }
  
  inline void set_mirroring(NES_PPU::mirroring_type m) 
  {
    attached_ppu->set_mirroring(m);
  }
};
/////////////////////////////////////////////////////////////////////

#endif
