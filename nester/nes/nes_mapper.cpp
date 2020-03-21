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

#define _NES_MAPPER_CPP_

#include "nester.h"
#include "nes.h"
#include "nes_mapper.h"

#include "debug.h"

/////////////////////////////////////////////////////////////////////
// Mapper virtual base class
bool
NES_mapper::initialize(NES* parent, NES_PPU *ppu)
{
  parent_NES = parent;
  attached_ppu = ppu;
  
  //num_16k_ROM_banks = parent_NES->ROM->get_num_16k_ROM_banks();
  num_8k_ROM_banks = 2 * parent_NES->ROM->get_num_16k_ROM_banks();
  num_1k_VROM_banks = 8 * parent_NES->ROM->get_num_8k_VROM_banks();
  
  ROM_banks  = parent_NES->ROM->get_ROM_banks();
  VROM_banks = parent_NES->ROM->get_VROM_banks();
  
  ROM_mask = num_8k_ROM_banks - 1;
  VROM_mask = num_1k_VROM_banks - 1;
  
  return true;
}

void NES_mapper::set_CPU_banks(uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  nes6502_context *context;
  
  context = nes6502_get_current_context ();
  context->mem_page[4] = ROM_banks + ((bank4_num & ROM_mask) << 13); // * 0x2000
  context->mem_page[5] = ROM_banks + ((bank5_num & ROM_mask) << 13);
  context->mem_page[6] = ROM_banks + ((bank6_num & ROM_mask) << 13);
  context->mem_page[7] = ROM_banks + ((bank7_num & ROM_mask) << 13);
  nes6502_setcontext();
}

// for mapper 40 /////////////////////////////////////////////////////////
void NES_mapper::set_CPU_banks(uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  nes6502_context *context;
  
  context = nes6502_get_current_context ();
  context->mem_page[3] = ROM_banks + ((bank3_num & ROM_mask) << 13); // * 0x2000
  context->mem_page[4] = ROM_banks + ((bank4_num & ROM_mask) << 13);
  context->mem_page[5] = ROM_banks + ((bank5_num & ROM_mask) << 13);
  context->mem_page[6] = ROM_banks + ((bank6_num & ROM_mask) << 13);
  context->mem_page[7] = ROM_banks + ((bank7_num & ROM_mask) << 13);
  nes6502_setcontext();
}


//////////////////////////////////////////////////////////////////////////
void NES_mapper::set_PPU_banks(uint32 bank0_num, uint32 bank1_num,
                               uint32 bank2_num, uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  attached_ppu->PPU_VRAM_banks[0] = VROM_banks + ((bank0_num & VROM_mask) << 10); // * 0x400
  attached_ppu->PPU_VRAM_banks[1] = VROM_banks + ((bank1_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[2] = VROM_banks + ((bank2_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[3] = VROM_banks + ((bank3_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[4] = VROM_banks + ((bank4_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[5] = VROM_banks + ((bank5_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[6] = VROM_banks + ((bank6_num & VROM_mask) << 10);
  attached_ppu->PPU_VRAM_banks[7] = VROM_banks + ((bank7_num & VROM_mask) << 10);
}

