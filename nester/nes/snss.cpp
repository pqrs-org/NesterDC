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

#include "nester.h"

#include "types.h"
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "snss.h"
#include "nes_6502.h"
#include "nes.h"
#include "nes_mapper_all.h"

#include "libsnss.h"
#include "sys/param.h"

// these functions apply a SNSS block to the current emulated NES

void
NES_SNSS::adopt_BASR(SnssBaseBlock* block, NES* nes)
{
  // BASR - Base Registers
  nes6502_context *context;
  
  context = nes6502_get_current_context ();
  context->a_reg = block->regA;
  context->x_reg = block->regX;
  context->y_reg = block->regY;
  context->p_reg = block->regFlags;
  context->s_reg = block->regStack;
  context->pc_reg = block->regPc;
  
  context->int_pending = 0;
  context->jammed = 0;
  context->burn_cycles = 0;
  //context->dma_cycles = 0;
  nes6502_setcontext();
  
  // registers $2000 and $2001
  nes->MemoryWrite(0x2000, block->reg2000);
  nes->MemoryWrite(0x2001, block->reg2001);
  
  // RAM
  memcpy(nes->RAM, block->cpuRam, 0x800);

  // SPR-RAM
  memcpy(nes->ppu->spr_ram, block->spriteRam, 0x100);

  // PPU $2000-$2FFF (Name Tables/Attrib Tables)
  memcpy(nes->ppu->PPU_nametables, block->ppuRam, 4*0x400);

  // palettes
  memcpy(nes->ppu->bg_pal,  &block->palette[0x00], 0x10);
  memcpy(nes->ppu->spr_pal, &block->palette[0x10], 0x10);

  // mirroring
  nes->ppu->set_mirroring((uint32)block->mirrorState[0]&0x03,
                          (uint32)block->mirrorState[1]&0x03,
                          (uint32)block->mirrorState[2]&0x03,
                          (uint32)block->mirrorState[3]&0x03);

  // VRAM address
  nes->ppu->loopy_t = block->vramAddress;

  // OAM (spr) address
  nes->ppu->spr_ram_rw_ptr = block->spriteRamAddress;

  // tile X offset
  nes->ppu->loopy_x = block->tileXOffset;
}

void
NES_SNSS::adopt_VRAM(SnssVramBlock* block, NES* nes)
{
  // VRAM

  // read MAX 32KB
  memcpy(nes->ppu->get_patt(), &block->vram, block->vramSize);

  if(block->vramSize > 0x2000)
  {
    LOG("SNSS VRAM size greater than 8K; unsupported\n");
  }
}

void
NES_SNSS::adopt_SRAM(SnssSramBlock* block, NES* nes)
{
  // Save-RAM
  nes6502_context *context;

  // read SRAM
  context = nes6502_get_current_context ();
  memcpy(context->mem_page[3], block->sram, (block->sramSize <= 0x2000) ? block->sramSize : 0x2000);
  nes6502_setcontext();
  
  if(block->sramSize > 0x2000)
    LOG("SNSS SRAM size greater than 8K; unsupported\n");
}

void
NES_SNSS::adopt_MPRD(SnssMapperBlock* block, NES* nes)
{
  // Mapper Data

  // set PRG pages
  nes6502_context *context;
  context = nes6502_get_current_context ();
  context->mem_page[4] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[0] << 13);
  context->mem_page[5] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[1] << 13);
  context->mem_page[6] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[2] << 13);
  context->mem_page[7] = nes->ROM->get_ROM_banks() + ((uint32)block->prgPages[3] << 13);
  nes6502_setcontext();
  
  // set CHR pages
  for(uint8 i = 0; i < 8; i++)
  {
    if((block->chrPages[i] & 0x8000) || !nes->ROM->get_num_8k_VROM_banks())
    {
      // VRAM
      nes->ppu->PPU_VRAM_banks[i] = nes->ppu->get_patt() + ((uint32)(block->chrPages[i] & 0x1f) << 10);
    }
    else
    {
      // VROM
      nes->ppu->PPU_VRAM_banks[i] = nes->ROM->get_VROM_banks() + ((uint32)block->chrPages[i] << 10);
    }
  }

  // handle mapper-specific data
  switch(nes->ROM->get_mapper_num())
  {
    case 1:
      {
        NES_mapper1* mapper = (NES_mapper1*)nes->mapper;
        mapper1Data* mapper_data = (mapper1Data*)&block->extraData;

        // last values written to the 4 registers
        memcpy(mapper->regs, mapper_data->registers, 4);

        // latch register
        mapper->bits = mapper_data->latch;

        // number of bits written to unfinished reg
        mapper->write_count = mapper_data->numberOfBits;
      }
      break;

    case 4:
      {
        NES_mapper4* mapper = (NES_mapper4*)nes->mapper;
        mapper4Data* mapper_data = (mapper4Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->last8000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 5:
      {
        NES_mapper5* mapper = (NES_mapper5*)nes->mapper;
        mapper5Data* mapper_data = (mapper5Data*)&block->extraData;

        mapper->irq_line = mapper_data->irqLineLowByte;
        mapper->irq_line |= mapper_data->irqLineHighByte << 8;
        mapper->irq_enabled = mapper_data->irqEnabled;
        mapper->irq_status = mapper_data->irqStatus;
        mapper->wram_protect0 = mapper_data->wramWriteProtect0;
        mapper->wram_protect1 = mapper_data->wramWriteProtect1;
        mapper->prg_size = mapper_data->romBankSize;
        mapper->chr_size = mapper_data->vromBankSize;
        mapper->gfx_mode = mapper_data->gfxMode;
        mapper->split_control = mapper_data->splitControl;
        mapper->split_bank = mapper_data->splitBank;
        mapper->value0 = mapper_data->last5205Write;
        mapper->value1 = mapper_data->last5206Write;
        mapper->wb[3] = mapper_data->wramBank3;
        mapper->wb[4] = mapper_data->wramBank4;
        mapper->wb[5] = mapper_data->wramBank5;
        mapper->wb[6] = mapper_data->wramBank6;
        mapper->chr_reg[0][0] = mapper_data->vromBank[0][0];
        mapper->chr_reg[1][0] = mapper_data->vromBank[1][0];
        mapper->chr_reg[2][0] = mapper_data->vromBank[2][0];
        mapper->chr_reg[3][0] = mapper_data->vromBank[3][0];
        mapper->chr_reg[4][0] = mapper_data->vromBank[4][0];
        mapper->chr_reg[5][0] = mapper_data->vromBank[5][0];
        mapper->chr_reg[6][0] = mapper_data->vromBank[6][0];
        mapper->chr_reg[7][0] = mapper_data->vromBank[7][0];
        mapper->chr_reg[0][1] = mapper_data->vromBank[0][1];
        mapper->chr_reg[1][1] = mapper_data->vromBank[1][1];
        mapper->chr_reg[2][1] = mapper_data->vromBank[2][1];
        mapper->chr_reg[3][1] = mapper_data->vromBank[3][1];
        mapper->chr_reg[4][1] = mapper_data->vromBank[4][1];
        mapper->chr_reg[5][1] = mapper_data->vromBank[5][1];
        mapper->chr_reg[6][1] = mapper_data->vromBank[6][1];
        mapper->chr_reg[7][1] = mapper_data->vromBank[7][1];
      }
      break;

    case 6:
      {
        NES_mapper6* mapper = (NES_mapper6*)nes->mapper;
        mapper6Data* mapper_data = (mapper6Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 9:
      {
        NES_mapper9* mapper = (NES_mapper9*)nes->mapper;
        mapper9Data* mapper_data = (mapper9Data*)&block->extraData;

        // 2 latch registers
        mapper->latch_0000 = mapper_data->latch[0];
        mapper->latch_1000 = mapper_data->latch[1];

        // regs (B/C/D/E000)
        mapper->regs[1] = mapper_data->lastB000Write;
        mapper->regs[2] = mapper_data->lastC000Write;
        mapper->regs[3] = mapper_data->lastD000Write;
        mapper->regs[4] = mapper_data->lastE000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 10:
      {
        NES_mapper10* mapper = (NES_mapper10*)nes->mapper;
        mapper10Data* mapper_data = (mapper10Data*)&block->extraData;

        // 2 latch registers
        mapper->latch_0000 = mapper_data->latch[0];
        mapper->latch_1000 = mapper_data->latch[1];

        // regs (B/C/D/E000)
        mapper->regs[1] = mapper_data->lastB000Write;
        mapper->regs[2] = mapper_data->lastC000Write;
        mapper->regs[3] = mapper_data->lastD000Write;
        mapper->regs[4] = mapper_data->lastE000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 13:
      {
        NES_mapper13* mapper = (NES_mapper13*)nes->mapper;
        mapper13Data* mapper_data = (mapper13Data*)&block->extraData;

        mapper->chr_bank = mapper_data->wramBank;

        mapper->SNSS_fixup();
      }
      break;

    case 16:
      {
        NES_mapper16* mapper = (NES_mapper16*)nes->mapper;
        mapper16Data* mapper_data = (mapper16Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounterLowByte;
        mapper->irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
      }
      break;

    case 17:
      {
        NES_mapper17* mapper = (NES_mapper17*)nes->mapper;
        mapper17Data* mapper_data = (mapper17Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 18:
      {
        NES_mapper18* mapper = (NES_mapper18*)nes->mapper;
        mapper18Data* mapper_data = (mapper18Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;

        mapper->SNSS_fixup();
      }
      break;

    case 19:
      {
        NES_mapper19* mapper = (NES_mapper19*)nes->mapper;
        mapper19Data* mapper_data = (mapper19Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = (mapper_data->lastE800Write & 0x40) >> 6;
        mapper->regs[1] = (mapper_data->lastE800Write & 0x80) >> 7;
        mapper->regs[2] = mapper_data->lastF800Write;
      }
      break;

    case 20:
      {
        NES_mapper20* mapper = (NES_mapper20*)nes->mapper;
        mapper20Data* mapper_data = (mapper20Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounterLowByte;
        mapper->irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
        mapper->irq_wait = mapper_data->irqWaitCounter;
        mapper->disk_enabled = mapper_data->last4023Write;
        mapper->write_reg = mapper_data->last4025Write;
        mapper->head_position = mapper_data->HeadPositionLowByte;
        mapper->head_position |= mapper_data->HeadPositionHighByte << 8;
        mapper->disk_status = mapper_data->DiskStatus;
        mapper->write_skip = mapper_data->WriteSkip;
        mapper->current_side = mapper_data->CurrentDiskSide;
        mapper->last_side = mapper_data->LastDiskSide;
        mapper->insert_wait = mapper_data->DiskInsertWait;

        mapper->SNSS_fixup();
      }
      break;

    case 21:
      {
        NES_mapper21* mapper = (NES_mapper21*)nes->mapper;
        mapper21Data* mapper_data = (mapper21Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->regs[8] = mapper_data->last9002Write;

        mapper->SNSS_fixup();
      }
      break;

    case 23:
      {
        NES_mapper23* mapper = (NES_mapper23*)nes->mapper;
        mapper23Data* mapper_data = (mapper23Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->regs[8] = mapper_data->last9008Write;

        mapper->SNSS_fixup();
      }
      break;

    case 24:
      {
        NES_mapper24* mapper = (NES_mapper24*)nes->mapper;
        mapper24Data* mapper_data = (mapper24Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled_l = mapper_data->irqCounterEnabled & 0x1;
        mapper->irq_enabled_h = mapper_data->irqCounterEnabled & 0x2;
        mapper->irq_latch = mapper_data->irqLatchCounter;
      }
      break;

    case 25:
      {
        NES_mapper25* mapper = (NES_mapper25*)nes->mapper;
        mapper25Data* mapper_data = (mapper25Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->regs[10] = mapper_data->last9001Write;

        mapper->SNSS_fixup();
      }
      break;

    case 26:
      {
        NES_mapper26* mapper = (NES_mapper26*)nes->mapper;
        mapper26Data* mapper_data = (mapper26Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
      }
      break;

    case 32:
      {
        NES_mapper32* mapper = (NES_mapper32*)nes->mapper;
        mapper32Data* mapper_data = (mapper32Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last9000Write;
      }
      break;

    case 33:
      {
        NES_mapper33* mapper = (NES_mapper33*)nes->mapper;
        mapper33Data* mapper_data = (mapper33Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 40:
      {
        NES_mapper40* mapper = (NES_mapper40*)nes->mapper;
        mapper40Data* mapper_data = (mapper40Data*)&block->extraData;

        // IRQ counter
        mapper->lines_to_irq = mapper_data->irqCounter;

        // IRQ enabled
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 41:
      {
        NES_mapper41* mapper = (NES_mapper41*)nes->mapper;
        mapper41Data* mapper_data = (mapper41Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last6000Write;
      }
      break;

    case 42:
      {
        NES_mapper42* mapper = (NES_mapper42*)nes->mapper;
        mapper42Data* mapper_data = (mapper42Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 43:
      {
        NES_mapper43* mapper = (NES_mapper43*)nes->mapper;
        mapper43Data* mapper_data = (mapper43Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 46:
      {
        NES_mapper46* mapper = (NES_mapper46*)nes->mapper;
        mapper46Data* mapper_data = (mapper46Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last6000Write & 0x0F;
        mapper->regs[1] = (mapper_data->last6000Write & 0xF0) >> 4;
        mapper->regs[2] = mapper_data->last8000Write & 0x01;
        mapper->regs[3] = (mapper_data->last8000Write & 0x70) >> 4;
      }
      break;

    case 48:
      {
        NES_mapper48* mapper = (NES_mapper48*)nes->mapper;
        mapper48Data* mapper_data = (mapper48Data*)&block->extraData;

        mapper->regs[0] = mapper_data->lastE000Write;
      }
      break;

    case 49:
      {
        NES_mapper49* mapper = (NES_mapper49*)nes->mapper;
        mapper49Data* mapper_data = (mapper49Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->last8000Write;
        mapper->regs[1] = mapper_data->last6000Write;
        mapper->regs[2] = mapper_data->lastA001Write;

        mapper->SNSS_fixup();
      }
      break;

    case 50:
      {
        NES_mapper50* mapper = (NES_mapper50*)nes->mapper;
        mapper50Data* mapper_data = (mapper50Data*)&block->extraData;

        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 51:
      {
        NES_mapper51* mapper = (NES_mapper51*)nes->mapper;
        mapper51Data* mapper_data = (mapper51Data*)&block->extraData;

        mapper->bank = mapper_data->BankSelect;
        mapper->mode = mapper_data->MapperMode;
      }
      break;

    case 57:
      {
        NES_mapper57* mapper = (NES_mapper57*)nes->mapper;
        mapper57Data* mapper_data = (mapper57Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last8800Write;
      }
      break;

    case 64:
      {
        NES_mapper64* mapper = (NES_mapper64*)nes->mapper;
        mapper64Data* mapper_data = (mapper64Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->regs[0] = mapper_data->last8000Write & 0x0F;
        mapper->regs[1] = mapper_data->last8000Write & 0x40;
        mapper->regs[2] = mapper_data->last8000Write & 0x80;
      }
      break;

    case 65:
      {
        NES_mapper65* mapper = (NES_mapper65*)nes->mapper;
        mapper65Data* mapper_data = (mapper65Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounterLowByte;
        mapper->irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
      }
      break;

    case 67:
      {
        NES_mapper67* mapper = (NES_mapper67*)nes->mapper;
        mapper67Data* mapper_data = (mapper67Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
      }
      break;

    case 68:
      {
        NES_mapper68* mapper = (NES_mapper68*)nes->mapper;
        mapper68Data* mapper_data = (mapper68Data*)&block->extraData;

        mapper->regs[0] = (mapper_data->lastE000Write & 0x10) >> 4;
        mapper->regs[1] = mapper_data->lastE000Write & 0x03;
        mapper->regs[2] = mapper_data->lastC000Write;
        mapper->regs[3] = mapper_data->lastD000Write;
      }
      break;

    case 69:
      {
        NES_mapper69* mapper = (NES_mapper69*)nes->mapper;
        mapper69Data* mapper_data = (mapper69Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->last8000Write;
      }
      break;

    case 73:
      {
        NES_mapper73* mapper = (NES_mapper73*)nes->mapper;
        mapper73Data* mapper_data = (mapper73Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 75:
      {
        NES_mapper75* mapper = (NES_mapper75*)nes->mapper;
        mapper75Data* mapper_data = (mapper75Data*)&block->extraData;

        mapper->regs[0] = mapper_data->lastE000Write;
        mapper->regs[1] = mapper_data->lastF000Write;
      }
      break;

    case 76:
      {
        NES_mapper76* mapper = (NES_mapper76*)nes->mapper;
        mapper76Data* mapper_data = (mapper76Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last8000Write;
      }
      break;

    case 82:
      {
        NES_mapper82* mapper = (NES_mapper82*)nes->mapper;
        mapper82Data* mapper_data = (mapper82Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last7EF6Write & 0x02;
      }
      break;

    case 83:
      {
        NES_mapper83* mapper = (NES_mapper83*)nes->mapper;
        mapper83Data* mapper_data = (mapper83Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounterLowByte;
        mapper->irq_counter |= mapper_data->irqCounterHighByte << 8;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->last8000Write;
        mapper->regs[1] = mapper_data->last8100Write;
        mapper->regs[2] = mapper_data->last5101Write;
      }
      break;

    case 85:
      {
        NES_mapper85* mapper = (NES_mapper85*)nes->mapper;
        mapper85Data* mapper_data = (mapper85Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
      }
      break;

    case 88:
      {
        NES_mapper88* mapper = (NES_mapper88*)nes->mapper;
        mapper88Data* mapper_data = (mapper88Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last8000Write;
      }
      break;

    case 91:
      {
        NES_mapper91* mapper = (NES_mapper91*)nes->mapper;
        mapper91Data* mapper_data = (mapper91Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
      }
      break;

    case 95:
      {
        NES_mapper95* mapper = (NES_mapper95*)nes->mapper;
        mapper95Data* mapper_data = (mapper95Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last8000Write;
        mapper->SNSS_fixup();
      }
      break;

    case 96:
      {
        NES_mapper96* mapper = (NES_mapper96*)nes->mapper;
        mapper96Data* mapper_data = (mapper96Data*)&block->extraData;

        mapper->vbank0 = mapper_data->wramBank;
      }
      break;

    case 105:
      {
        NES_mapper105* mapper = (NES_mapper105*)nes->mapper;
        mapper105Data* mapper_data = (mapper105Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter[0];
        mapper->irq_counter |= mapper_data->irqCounter[1] << 8;
        mapper->irq_counter |= mapper_data->irqCounter[2] << 16;
        mapper->irq_counter |= mapper_data->irqCounter[3] << 24;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->init_state = mapper_data->InitialCounter;
        mapper->write_count = mapper_data->WriteCounter;
        mapper->bits = mapper_data->Bits;
        mapper->regs[0] = mapper_data->registers[0];
        mapper->regs[1] = mapper_data->registers[1];
        mapper->regs[2] = mapper_data->registers[2];
        mapper->regs[3] = mapper_data->registers[3];
      }
      break;

    case 117:
      {
        NES_mapper117* mapper = (NES_mapper117*)nes->mapper;
        mapper117Data* mapper_data = (mapper117Data*)&block->extraData;

        mapper->irq_line = mapper_data->irqLine;
        mapper->irq_enabled1 = mapper_data->irqEnabled1;
        mapper->irq_enabled2 = mapper_data->irqEnabled2;
      }
      break;

    case 160:
      {
        NES_mapper160* mapper = (NES_mapper160*)nes->mapper;
        mapper160Data* mapper_data = (mapper160Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->refresh_type = mapper_data->RefreshType;
      }
      break;

    case 182:
      {
        NES_mapper182* mapper = (NES_mapper182*)nes->mapper;
        mapper182Data* mapper_data = (mapper182Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->lastA000Write;
      }
      break;

    case 189:
      {
        NES_mapper189* mapper = (NES_mapper189*)nes->mapper;
        mapper189Data* mapper_data = (mapper189Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->regs[0] = mapper_data->last8000Write;
      }
      break;

    case 226:
      {
        NES_mapper226* mapper = (NES_mapper226*)nes->mapper;
        mapper226Data* mapper_data = (mapper226Data*)&block->extraData;

        mapper->regs[0] = mapper_data->registers[0];
        mapper->regs[1] = mapper_data->registers[1];
      }
      break;

    case 230:
      {
        NES_mapper230* mapper = (NES_mapper230*)nes->mapper;
        mapper230Data* mapper_data = (mapper230Data*)&block->extraData;

        mapper->rom_switch = mapper_data->numberOfResets;
      }
      break;

    case 232:
      {
        NES_mapper232* mapper = (NES_mapper232*)nes->mapper;
        mapper232Data* mapper_data = (mapper232Data*)&block->extraData;

        mapper->regs[0] = mapper_data->last9000Write;
        mapper->regs[1] = mapper_data->lastA000Write;
      }
      break;

    case 234:
      {
        NES_mapper234* mapper = (NES_mapper234*)nes->mapper;
        mapper234Data* mapper_data = (mapper234Data*)&block->extraData;

        mapper->regs[0] = mapper_data->lastFF80Write;
        mapper->regs[1] = mapper_data->lastFFE8Write;
        mapper->regs[2] = mapper_data->lastFFC0Write;
      }
      break;

    case 236:
      {
        NES_mapper236* mapper = (NES_mapper236*)nes->mapper;
        mapper236Data* mapper_data = (mapper236Data*)&block->extraData;

        mapper->bank = mapper_data->BankSelect;
        mapper->mode = mapper_data->MapperMode;
      }
      break;

    case 243:
      {
        NES_mapper243* mapper = (NES_mapper243*)nes->mapper;
        mapper243Data* mapper_data = (mapper243Data*)&block->extraData;

        mapper->regs[0] = mapper_data->registers[0];
        mapper->regs[1] = mapper_data->registers[1];
        mapper->regs[2] = mapper_data->registers[2];
        mapper->regs[3] = mapper_data->registers[3];
      }
      break;

    case 248:
      {
        NES_mapper248* mapper = (NES_mapper248*)nes->mapper;
        mapper248Data* mapper_data = (mapper248Data*)&block->extraData;

        mapper->irq_counter = mapper_data->irqCounter;
        mapper->irq_latch = mapper_data->irqLatchCounter;
        mapper->irq_enabled = mapper_data->irqCounterEnabled;
        mapper->regs[0] = mapper_data->last8000Write;

        mapper->SNSS_fixup();
      }
      break;

    case 255:
      {
        NES_mapper255* mapper = (NES_mapper255*)nes->mapper;
        mapper255Data* mapper_data = (mapper255Data*)&block->extraData;

        mapper->regs[0] = mapper_data->registers[0];
        mapper->regs[1] = mapper_data->registers[1];
        mapper->regs[2] = mapper_data->registers[2];
        mapper->regs[3] = mapper_data->registers[3];
      }
      break;
  }
}


static void 
analyze_blockheader(SnssBlockHeader *sbh, uint8 *header)
{
  uint32 v, s;
  
  memset(sbh, 0, sizeof(SnssBlockHeader));
  memcpy(sbh->tag, header, 4);
  memcpy(&v, header + 4, 4);
  memcpy(&s, header + 8, 4);
  
  sbh->blockVersion = ntohl(v);
  sbh->blockLength = ntohl(s);
}

void
NES_SNSS::adopt_ExMPRD(SNSS_FILE *snssFile, NES* nes)
{
  uint32 mapper_num = nes->ROM->get_mapper_num();
  
  if (mapper_num == 1 && nes->crc32() == 0xb8747abf)
  {
    // Best Play - Pro Yakyuu Special (J)
    
    // save WRAM
    fseek(snssFile->fp, 12, SEEK_CUR);
    fread (nes->SaveRAM + 0x2000, 0x2000, 1, snssFile->fp);
  }
  else if (mapper_num == 5)
  {
    NES_mapper5* mapper = (NES_mapper5*)nes->mapper;
    
    // read WRAM
    fseek(snssFile->fp, 12, SEEK_CUR);
    fread (mapper->wram, 0x10000, 1, snssFile->fp);
    memcpy (nes->SaveRAM, mapper->wram, 0x10000);
  }
  else if (mapper_num == 20)
  {
    NES_mapper20* mapper = (NES_mapper20*)nes->mapper;
    uint8 header[12];
    uint8 buf[NES::fds_save_buflen];
    
    fread(header, 1, 12, snssFile->fp);
    if (memcmp(header, "WRAM", 4)) 
      goto error;
    fread(mapper->wram, 1, 0x8000, snssFile->fp);
    
    fread(header, 1, 12, snssFile->fp);
    if (memcmp(header, "FDSD", 4))
      goto error;
    if (fread(buf, sizeof(buf), 1, snssFile->fp) != 1)
      goto error;
    
    nes->restore_savedata(buf);
  }
  
  return;
  
error:
  ERROR("error in adopt_ExMPRD");
}

void 
NES_SNSS::adopt_CNTR(SnssControllersBlock* block, NES* nes)
{
}

void
NES_SNSS::adopt_SOUN(SnssSoundBlock* block, NES* nes)
{
  // Sound Data

  // give them to the apu
  nes->apu->load_regs(block->soundRegisters);
}

boolean
NES_SNSS::LoadSNSS(const char* fn, NES* nes)
{
  SNSS_FILE* snssFile = NULL;
  SNSS_BLOCK_TYPE blockType;
  
  if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_READ))
    return false;
  
  // at this point, it's too late to go back, and the NES must be reset on failure
  for(int i = 0; i < (int)snssFile->headerBlock.numberOfBlocks; i++)
  {
    if(SNSS_OK != SNSS_GetNextBlockType(&blockType, snssFile))
    {
      ERROR("error SNSS_GetNextBlockType");
      goto error;
    }
    
    if(SNSS_OK != SNSS_ReadBlock(snssFile, blockType))
    {
      ERROR("error SNSS_ReadBlock");
      goto error;
    }
    
    switch(blockType) 
    {
      case SNSS_BASR:
	adopt_BASR(&snssFile->baseBlock, nes);
	break;
	
      case SNSS_VRAM:
	adopt_VRAM(&snssFile->vramBlock, nes);
	break;
	
      case SNSS_SRAM:
	adopt_SRAM(&snssFile->sramBlock, nes);
	break;
	
      case SNSS_MPRD:
	adopt_MPRD(&snssFile->mapperBlock, nes);
	break;
	
      case SNSS_CNTR:
	adopt_CNTR(&snssFile->contBlock, nes);
	break;
	
      case SNSS_SOUN:
	adopt_SOUN(&snssFile->soundBlock, nes);
	break;
	
      case SNSS_UNKNOWN_BLOCK:
	break;
	
      default:
	ERROR("invalid blockType");
	goto error;
    }
  }
  
  // read Extra Mapper Data
  adopt_ExMPRD(snssFile, nes);
  
  SNSS_CloseFile(&snssFile);
  
  return TRUE;
  
error:
  if(snssFile) SNSS_CloseFile(&snssFile);
  nes->reset();
  return FALSE;
}


// these functions create a SNSS block from the current emulated NES
// return 0 if block is valid

int
NES_SNSS::extract_BASR(SnssBaseBlock* block, NES* nes)
{
  nes6502_context *context;
  
  // get the CPU context
  context = nes6502_get_current_context ();
  // CPU data
  block->regA = context->a_reg;
  block->regX = context->x_reg;
  block->regY = context->y_reg;
  block->regFlags = context->p_reg;
  block->regStack = context->s_reg;
  block->regPc = context->pc_reg;
  
  // $2000 and $2001
  block->reg2000 = nes->ppu->LowRegs[0];
  block->reg2001 = nes->ppu->LowRegs[1];

  // RAM
  memcpy(block->cpuRam, nes->RAM, 0x800);

  // SPR-RAM
  memcpy(block->spriteRam, nes->ppu->spr_ram, 0x100);

  // PPU $2000-$2FFF (Name Tables/Attrib Tables)
  memcpy(block->ppuRam, nes->ppu->PPU_nametables, 4*0x400);

  // palettes
  memcpy(block->palette + 0x00, nes->ppu->bg_pal,  0x10);
  memcpy(block->palette + 0x10, nes->ppu->spr_pal, 0x10);
  
  // mirroring
  block->mirrorState[0] = (nes->ppu->PPU_VRAM_banks[0x08] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[1] = (nes->ppu->PPU_VRAM_banks[0x09] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[2] = (nes->ppu->PPU_VRAM_banks[0x0A] - nes->ppu->PPU_nametables) >> 10;
  block->mirrorState[3] = (nes->ppu->PPU_VRAM_banks[0x0B] - nes->ppu->PPU_nametables) >> 10;
  ASSERT(block->mirrorState[0] < 4); ASSERT(block->mirrorState[1] < 4);
  ASSERT(block->mirrorState[2] < 4); ASSERT(block->mirrorState[3] < 4);

  // VRAM address
  block->vramAddress = nes->ppu->loopy_t;

  // OAM (sprite) address
  block->spriteRamAddress = nes->ppu->spr_ram_rw_ptr;

  // tile X offset
  block->tileXOffset = nes->ppu->loopy_x;
  return 0;
}

int 
NES_SNSS::extract_VRAM(SnssVramBlock* block, NES* nes)
{
  // if cart has VROM, don't write any VRAM
  //if(nes->ROM->get_num_8k_VROM_banks()) return -1;
  
  uint8* patterntables = nes->ppu->get_patt();
  uint32 i;
  for(i = 0; i < 0x8000; i++) {
    if(patterntables[i] != 0x00) break;
  }
  if(i == 0x8000) return -1;
  
  // 8K of VRAM data
  if (nes->get_mapper_num() == 6)
    block->vramSize = 0x8000;
  else if (nes->get_mapper_num() == 13)
    block->vramSize = 0x4000;
  else if (nes->get_mapper_num() == 96)
    block->vramSize = 0x8000;
  else
    block->vramSize = 0x2000;
  
  memcpy(&block->vram, nes->ppu->get_patt(), block->vramSize);
  
  return 0;
}

int
NES_SNSS::extract_SRAM(SnssSramBlock* block, NES* nes)
{
  nes6502_context *context;
  
  if (nes->is_invalid_sram ()) 
    return -1; 
  
  // SRAM writeable flag
  block->sramEnabled = 1;
  
  // SRAM size (8k)
  block->sramSize = 0x2000;
  
  // SRAM data
  context = nes6502_get_current_context ();
  memcpy(block->sram, context->mem_page[3], 0x2000);
  
  return 0;
}

int 
NES_SNSS::extract_MPRD(SnssMapperBlock* block, NES* nes)
{
  nes6502_context *context;
  
  if (0 == nes->ROM->get_mapper_num()) return -1;
  
  // 8K PRG page numbers
  context = nes6502_get_current_context ();
  block->prgPages[0] = (context->mem_page[4] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[1] = (context->mem_page[5] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[2] = (context->mem_page[6] - nes->ROM->get_ROM_banks()) >> 13;
  block->prgPages[3] = (context->mem_page[7] - nes->ROM->get_ROM_banks()) >> 13;
  
  // 1K CHR page numbers
  for(uint8 i = 0; i < 8; i++) {
    if (nes->ppu->get_VRAM_bank_type (i)) {
      // VRAM
      block->chrPages[i] = ((nes->ppu->PPU_VRAM_banks[i] - nes->ppu->get_patt()) >> 10) | 0x8000;
    } else {
      // VROM
      block->chrPages[i] = (nes->ppu->PPU_VRAM_banks[i] - nes->ROM->get_VROM_banks()) >> 10;
    }
  }
  
  switch(nes->ROM->get_mapper_num())
  {
    case 1:
      {
        NES_mapper1* mapper = (NES_mapper1*)nes->mapper;
        mapper1Data* mapper_data = (mapper1Data*)&block->extraData;

        // last values written to the 4 registers
        memcpy(mapper_data->registers, mapper->regs, 4);

        // latch register
        mapper_data->latch = mapper->bits;

        // number of bits written to unfinished reg
        mapper_data->numberOfBits = mapper->write_count;
      }
      break;

    case 4:
      {
        NES_mapper4* mapper = (NES_mapper4*)nes->mapper;
        mapper4Data* mapper_data = (mapper4Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 5:
      {
        NES_mapper5* mapper = (NES_mapper5*)nes->mapper;
        mapper5Data* mapper_data = (mapper5Data*)&block->extraData;

        mapper_data->irqLineLowByte = mapper->irq_line & 0x00ff;
        mapper_data->irqLineHighByte = (mapper->irq_line & 0xff00) >> 8;
        mapper_data->irqEnabled = mapper->irq_enabled;
        mapper_data->irqStatus = mapper->irq_status;
        mapper_data->wramWriteProtect0 = mapper->wram_protect0;
        mapper_data->wramWriteProtect1 = mapper->wram_protect1;
        mapper_data->romBankSize = mapper->prg_size;
        mapper_data->vromBankSize = mapper->chr_size;
        mapper_data->gfxMode = mapper->gfx_mode;
        mapper_data->splitControl = mapper->split_control;
        mapper_data->splitBank = mapper->split_bank;
        mapper_data->last5205Write = mapper->value0;
        mapper_data->last5206Write = mapper->value1;
        mapper_data->wramBank3 = mapper->wb[3];
        mapper_data->wramBank4 = mapper->wb[4];
        mapper_data->wramBank5 = mapper->wb[5];
        mapper_data->wramBank6 = mapper->wb[6];
        mapper_data->vromBank[0][0] = mapper->chr_reg[0][0];
        mapper_data->vromBank[1][0] = mapper->chr_reg[1][0];
        mapper_data->vromBank[2][0] = mapper->chr_reg[2][0];
        mapper_data->vromBank[3][0] = mapper->chr_reg[3][0];
        mapper_data->vromBank[4][0] = mapper->chr_reg[4][0];
        mapper_data->vromBank[5][0] = mapper->chr_reg[5][0];
        mapper_data->vromBank[6][0] = mapper->chr_reg[6][0];
        mapper_data->vromBank[7][0] = mapper->chr_reg[7][0];
        mapper_data->vromBank[0][1] = mapper->chr_reg[0][1];
        mapper_data->vromBank[1][1] = mapper->chr_reg[1][1];
        mapper_data->vromBank[2][1] = mapper->chr_reg[2][1];
        mapper_data->vromBank[3][1] = mapper->chr_reg[3][1];
        mapper_data->vromBank[4][1] = mapper->chr_reg[4][1];
        mapper_data->vromBank[5][1] = mapper->chr_reg[5][1];
        mapper_data->vromBank[6][1] = mapper->chr_reg[6][1];
        mapper_data->vromBank[7][1] = mapper->chr_reg[7][1];
      }
      break;

    case 6:
      {
        NES_mapper6* mapper = (NES_mapper6*)nes->mapper;
        mapper6Data* mapper_data = (mapper6Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 9:
      {
        NES_mapper9* mapper = (NES_mapper9*)nes->mapper;
        mapper9Data* mapper_data = (mapper9Data*)&block->extraData;

        // 2 latch registers
        mapper_data->latch[0] = mapper->latch_0000;
        mapper_data->latch[1] = mapper->latch_1000;

        // regs (B/C/D/E000)
        mapper_data->lastB000Write = mapper->regs[1];
        mapper_data->lastC000Write = mapper->regs[2];
        mapper_data->lastD000Write = mapper->regs[3];
        mapper_data->lastE000Write = mapper->regs[4];
      }
      break;

    case 10:
      {
        NES_mapper10* mapper = (NES_mapper10*)nes->mapper;
        mapper10Data* mapper_data = (mapper10Data*)&block->extraData;

        // 2 latch registers
        mapper_data->latch[0] = mapper->latch_0000;
        mapper_data->latch[1] = mapper->latch_1000;

        // regs (B/C/D/E000)
        mapper_data->lastB000Write = mapper->regs[1];
        mapper_data->lastC000Write = mapper->regs[2];
        mapper_data->lastD000Write = mapper->regs[3];
        mapper_data->lastE000Write = mapper->regs[4];
      }
      break;

    case 13:
      {
        NES_mapper13* mapper = (NES_mapper13*)nes->mapper;
        mapper13Data* mapper_data = (mapper13Data*)&block->extraData;

        mapper_data->wramBank = mapper->chr_bank;
      }
      break;

    case 16:
      {
        NES_mapper16* mapper = (NES_mapper16*)nes->mapper;
        mapper16Data* mapper_data = (mapper16Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounterLowByte = mapper->irq_latch & 0x00FF;
        mapper_data->irqLatchCounterHighByte = (mapper->irq_latch & 0xFF00) >> 8;
      }
      break;

    case 17:
      {
        NES_mapper17* mapper = (NES_mapper17*)nes->mapper;
        mapper17Data* mapper_data = (mapper17Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 18:
      {
        NES_mapper18* mapper = (NES_mapper18*)nes->mapper;
        mapper18Data* mapper_data = (mapper18Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 19:
      {
        NES_mapper19* mapper = (NES_mapper19*)nes->mapper;
        mapper19Data* mapper_data = (mapper19Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->lastE800Write = (mapper->regs[0] & 0x01) << 6;
        mapper_data->lastE800Write |= (mapper->regs[1] & 0x01) << 7;
        mapper_data->lastF800Write = mapper->regs[2];
      }
      break;

    case 20:
      {
        NES_mapper20* mapper = (NES_mapper20*)nes->mapper;
        mapper20Data* mapper_data = (mapper20Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounterLowByte = mapper->irq_latch & 0x00FF;
        mapper_data->irqLatchCounterHighByte = (mapper->irq_latch & 0xFF00) >> 8;
        mapper_data->irqWaitCounter = mapper->irq_wait;
        mapper_data->last4023Write = mapper->disk_enabled;
        mapper_data->last4025Write = mapper->write_reg;
        mapper_data->HeadPositionLowByte = mapper->head_position & 0x00FF;
        mapper_data->HeadPositionHighByte = (mapper->head_position & 0xFF00) >> 8;
        mapper_data->DiskStatus = mapper->disk_status;
        mapper_data->WriteSkip = mapper->write_skip;
        mapper_data->CurrentDiskSide = mapper->current_side;
        mapper_data->LastDiskSide = mapper->last_side;
        mapper_data->DiskInsertWait = mapper->insert_wait;
      }
      break;

    case 21:
      {
        NES_mapper21* mapper = (NES_mapper21*)nes->mapper;
        mapper21Data* mapper_data = (mapper21Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->last9002Write = mapper->regs[8];
      }
      break;

    case 23:
      {
        NES_mapper23* mapper = (NES_mapper23*)nes->mapper;
        mapper23Data* mapper_data = (mapper23Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->last9008Write = mapper->regs[8];
      }
      break;

    case 24:
      {
        NES_mapper24* mapper = (NES_mapper24*)nes->mapper;
        mapper24Data* mapper_data = (mapper24Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled_h | mapper->irq_enabled_l;
        mapper_data->irqLatchCounter = mapper->irq_latch;
      }
      break;

    case 25:
      {
        NES_mapper25* mapper = (NES_mapper25*)nes->mapper;
        mapper25Data* mapper_data = (mapper25Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->last9001Write = mapper->regs[10];
      }
      break;

    case 26:
      {
        NES_mapper26* mapper = (NES_mapper26*)nes->mapper;
        mapper26Data* mapper_data = (mapper26Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
      }
      break;

    case 32:
      {
        NES_mapper32* mapper = (NES_mapper32*)nes->mapper;
        mapper32Data* mapper_data = (mapper32Data*)&block->extraData;

        mapper_data->last9000Write = mapper->regs[0];
      }
      break;

    case 33:
      {
        NES_mapper33* mapper = (NES_mapper33*)nes->mapper;
        mapper33Data* mapper_data = (mapper33Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 40:
      {
        NES_mapper40* mapper = (NES_mapper40*)nes->mapper;
        mapper40Data* mapper_data = (mapper40Data*)&block->extraData;

        // IRQ counter
        mapper_data->irqCounter = mapper->lines_to_irq;

        // IRQ enabled
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 41:
      {
        NES_mapper41* mapper = (NES_mapper41*)nes->mapper;
        mapper41Data* mapper_data = (mapper41Data*)&block->extraData;

        mapper_data->last6000Write = mapper->regs[0];
      }
      break;

    case 42:
      {
        NES_mapper42* mapper = (NES_mapper42*)nes->mapper;
        mapper42Data* mapper_data = (mapper42Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 43:
      {
        NES_mapper43* mapper = (NES_mapper43*)nes->mapper;
        mapper43Data* mapper_data = (mapper43Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 46:
      {
        NES_mapper46* mapper = (NES_mapper46*)nes->mapper;
        mapper46Data* mapper_data = (mapper46Data*)&block->extraData;

        mapper_data->last6000Write = mapper->regs[0] | (mapper->regs[1] << 4);
        mapper_data->last8000Write = mapper->regs[2] | (mapper->regs[1] << 4);
      }
      break;

    case 48:
      {
        NES_mapper48* mapper = (NES_mapper48*)nes->mapper;
        mapper48Data* mapper_data = (mapper48Data*)&block->extraData;

        mapper_data->lastE000Write = mapper->regs[0];
      }
      break;

    case 49:
      {
        NES_mapper49* mapper = (NES_mapper49*)nes->mapper;
        mapper49Data* mapper_data = (mapper49Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->last8000Write = mapper->regs[0];
        mapper_data->last6000Write = mapper->regs[1];
        mapper_data->lastA001Write = mapper->regs[2];
      }
      break;

    case 50:
      {
        NES_mapper50* mapper = (NES_mapper50*)nes->mapper;
        mapper50Data* mapper_data = (mapper50Data*)&block->extraData;

        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 51:
      {
        NES_mapper51* mapper = (NES_mapper51*)nes->mapper;
        mapper51Data* mapper_data = (mapper51Data*)&block->extraData;

        mapper_data->BankSelect = mapper->bank;
        mapper_data->MapperMode = mapper->mode;
      }
      break;

    case 57:
      {
        NES_mapper57* mapper = (NES_mapper57*)nes->mapper;
        mapper57Data* mapper_data = (mapper57Data*)&block->extraData;

        mapper_data->last8800Write = mapper->regs[0];
      }
      break;

    case 64:
      {
        NES_mapper64* mapper = (NES_mapper64*)nes->mapper;
        mapper64Data* mapper_data = (mapper64Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->last8000Write = mapper->regs[0] | mapper->regs[1] | mapper->regs[2];
      }
      break;

    case 65:
      {
        NES_mapper65* mapper = (NES_mapper65*)nes->mapper;
        mapper65Data* mapper_data = (mapper65Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounterLowByte = mapper->irq_latch & 0x00FF;
        mapper_data->irqLatchCounterHighByte = (mapper->irq_latch & 0xFF00) >> 8;
      }
      break;

    case 67:
      {
        NES_mapper67* mapper = (NES_mapper67*)nes->mapper;
        mapper67Data* mapper_data = (mapper67Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
      }
      break;

    case 68:
      {
        NES_mapper68* mapper = (NES_mapper68*)nes->mapper;
        mapper68Data* mapper_data = (mapper68Data*)&block->extraData;

        mapper_data->lastC000Write = mapper->regs[2];
        mapper_data->lastD000Write = mapper->regs[3];
        mapper_data->lastE000Write = (mapper->regs[0] << 4) | mapper->regs[1];
      }
      break;

    case 69:
      {
        NES_mapper69* mapper = (NES_mapper69*)nes->mapper;
        mapper69Data* mapper_data = (mapper69Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 73:
      {
        NES_mapper73* mapper = (NES_mapper73*)nes->mapper;
        mapper73Data* mapper_data = (mapper73Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 75:
      {
        NES_mapper75* mapper = (NES_mapper75*)nes->mapper;
        mapper75Data* mapper_data = (mapper75Data*)&block->extraData;

        mapper_data->lastE000Write = mapper->regs[0];
        mapper_data->lastF000Write = mapper->regs[1];
      }
      break;

    case 76:
      {
        NES_mapper76* mapper = (NES_mapper76*)nes->mapper;
        mapper76Data* mapper_data = (mapper76Data*)&block->extraData;

        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 82:
      {
        NES_mapper82* mapper = (NES_mapper82*)nes->mapper;
        mapper82Data* mapper_data = (mapper82Data*)&block->extraData;

        mapper_data->last7EF6Write = mapper->regs[0];
      }
      break;

    case 83:
      {
        NES_mapper83* mapper = (NES_mapper83*)nes->mapper;
        mapper83Data* mapper_data = (mapper83Data*)&block->extraData;

        mapper_data->irqCounterLowByte = mapper->irq_counter & 0x00FF;
        mapper_data->irqCounterHighByte = (mapper->irq_counter & 0xFF00) >> 8;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->last8000Write = mapper->regs[0];
        mapper_data->last8100Write = mapper->regs[1];
        mapper_data->last5101Write = mapper->regs[2];
      }
      break;

    case 85:
      {
        NES_mapper85* mapper = (NES_mapper85*)nes->mapper;
        mapper85Data* mapper_data = (mapper85Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
      }
      break;

    case 88:
      {
        NES_mapper88* mapper = (NES_mapper88*)nes->mapper;
        mapper88Data* mapper_data = (mapper88Data*)&block->extraData;

        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 91:
      {
        NES_mapper91* mapper = (NES_mapper91*)nes->mapper;
        mapper91Data* mapper_data = (mapper91Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
      }
      break;

    case 95:
      {
        NES_mapper95* mapper = (NES_mapper95*)nes->mapper;
        mapper95Data* mapper_data = (mapper95Data*)&block->extraData;

        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 96:
      {
        NES_mapper96* mapper = (NES_mapper96*)nes->mapper;
        mapper96Data* mapper_data = (mapper96Data*)&block->extraData;

        mapper_data->wramBank = mapper->vbank0;
      }
      break;

    case 105:
      {
        NES_mapper105* mapper = (NES_mapper105*)nes->mapper;
        mapper105Data* mapper_data = (mapper105Data*)&block->extraData;

        mapper_data->irqCounter[0] = (mapper->irq_counter &0x000000ff) >> 0;
        mapper_data->irqCounter[1] = (mapper->irq_counter &0x0000ff00) >> 8;
        mapper_data->irqCounter[2] = (mapper->irq_counter &0x00ff0000) >> 16;
        mapper_data->irqCounter[3] = (mapper->irq_counter &0xff000000) >> 24;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->InitialCounter = mapper->init_state;
        mapper_data->WriteCounter = mapper->write_count;
        mapper_data->Bits = mapper->bits;
        mapper_data->registers[0] = mapper->regs[0];
        mapper_data->registers[1] = mapper->regs[1];
        mapper_data->registers[2] = mapper->regs[2];
        mapper_data->registers[3] = mapper->regs[3];
      }
      break;

    case 117:
      {
        NES_mapper117* mapper = (NES_mapper117*)nes->mapper;
        mapper117Data* mapper_data = (mapper117Data*)&block->extraData;

        mapper_data->irqLine = mapper->irq_line;
        mapper_data->irqEnabled1 = mapper->irq_enabled1;
        mapper_data->irqEnabled2 = mapper->irq_enabled2;
      }
      break;

    case 160:
      {
        NES_mapper160* mapper = (NES_mapper160*)nes->mapper;
        mapper160Data* mapper_data = (mapper160Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->RefreshType = mapper->refresh_type;
      }
      break;

    case 182:
      {
        NES_mapper182* mapper = (NES_mapper182*)nes->mapper;
        mapper182Data* mapper_data = (mapper182Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->lastA000Write = mapper->regs[0];
      }
      break;

    case 189:
      {
        NES_mapper189* mapper = (NES_mapper189*)nes->mapper;
        mapper189Data* mapper_data = (mapper189Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 226:
      {
        NES_mapper226* mapper = (NES_mapper226*)nes->mapper;
        mapper226Data* mapper_data = (mapper226Data*)&block->extraData;

        mapper_data->registers[0] = mapper->regs[0];
        mapper_data->registers[1] = mapper->regs[1];
      }
      break;

    case 230:
      {
        NES_mapper230* mapper = (NES_mapper230*)nes->mapper;
        mapper230Data* mapper_data = (mapper230Data*)&block->extraData;

        mapper_data->numberOfResets = mapper->rom_switch;
      }
      break;

    case 232:
      {
        NES_mapper232* mapper = (NES_mapper232*)nes->mapper;
        mapper232Data* mapper_data = (mapper232Data*)&block->extraData;

        mapper_data->last9000Write = mapper->regs[0];
        mapper_data->lastA000Write = mapper->regs[1];
      }
      break;

    case 234:
      {
        NES_mapper234* mapper = (NES_mapper234*)nes->mapper;
        mapper234Data* mapper_data = (mapper234Data*)&block->extraData;

        mapper_data->lastFF80Write = mapper->regs[0];
        mapper_data->lastFFE8Write = mapper->regs[1];
        mapper_data->lastFFC0Write = mapper->regs[2];
      }
      break;

    case 236:
      {
        NES_mapper236* mapper = (NES_mapper236*)nes->mapper;
        mapper236Data* mapper_data = (mapper236Data*)&block->extraData;

        mapper_data->BankSelect = mapper->bank;
        mapper_data->MapperMode = mapper->mode;
      }
      break;

    case 243:
      {
        NES_mapper243* mapper = (NES_mapper243*)nes->mapper;
        mapper243Data* mapper_data = (mapper243Data*)&block->extraData;

        mapper_data->registers[0] = mapper->regs[0];
        mapper_data->registers[1] = mapper->regs[1];
        mapper_data->registers[2] = mapper->regs[2];
        mapper_data->registers[3] = mapper->regs[3];
      }
      break;

    case 248:
      {
        NES_mapper248* mapper = (NES_mapper248*)nes->mapper;
        mapper248Data* mapper_data = (mapper248Data*)&block->extraData;

        mapper_data->irqCounter = mapper->irq_counter;
        mapper_data->irqLatchCounter = mapper->irq_latch;
        mapper_data->irqCounterEnabled = mapper->irq_enabled;
        mapper_data->last8000Write = mapper->regs[0];
      }
      break;

    case 255:
      {
        NES_mapper255* mapper = (NES_mapper255*)nes->mapper;
        mapper255Data* mapper_data = (mapper255Data*)&block->extraData;

        mapper_data->registers[0] = mapper->regs[0];
        mapper_data->registers[1] = mapper->regs[1];
        mapper_data->registers[2] = mapper->regs[2];
        mapper_data->registers[3] = mapper->regs[3];
      }
      break;
  }

  return 0;
}


static void
make_blockheader(uint8 *header, 
		 const char *signature, uint32 version, uint32 size)
{
  uint32 v, s;
  
  v = htonl(version);
  s = htonl(size);
  
  memset(header, 0, 12);
  memcpy(header, signature, 4);
  memcpy(header + 4, &v, 4);
  memcpy(header + 8, &s, 4);
}

void
NES_SNSS::extract_ExMPRD(SNSS_FILE *snssFile, NES* nes)
{
  uint32 mapper_num = nes->ROM->get_mapper_num();
  
  if(mapper_num == 1 && nes->crc32() == 0xb8747abf)
  {
    // Best Play - Pro Yakyuu Special (J)
    uint8 header[12];
    
    make_blockheader(header, "WRAM", 1, 0x2000);
    fwrite(header, 1, 12, snssFile->fp);
    fwrite(nes->SaveRAM + 0x2000, 1, 0x2000, snssFile->fp);
  }
  else if(mapper_num == 5)
  {
    NES_mapper5* mapper = (NES_mapper5*)nes->mapper;
    uint8 header[12];
    
    make_blockheader(header, "WRAM", 1, 0x10000);
    fwrite(header, 1, 12, snssFile->fp);
    fwrite(mapper->wram, 1, 0x10000, snssFile->fp);
  }
  else if(mapper_num == 20)
  {
    NES_mapper20* mapper = (NES_mapper20*)nes->mapper;
    uint8 header[12];
    uint8 buf[NES::fds_save_buflen];
    
    make_blockheader(header, "WRAM", 1, 0x8000);
    fwrite(header, 1, 12, snssFile->fp);
    fwrite(mapper->wram, 1, 0x8000, snssFile->fp); 
    
    nes->make_savedata (buf);
    
    make_blockheader(header, "FDSD", 1, 65500);
    fwrite(header, 1, 12, snssFile->fp);
    fwrite(buf, 1, nes->fds_save_buflen, snssFile->fp);
  }
}

int
NES_SNSS::extract_CNTR(SnssControllersBlock* block, NES* nes)
{
  return -1;
}

int
NES_SNSS::extract_SOUN(SnssSoundBlock* block, NES* nes)
{
  // get sound registers
  nes->apu->get_regs(block->soundRegisters);
  
  return 0;
}

boolean 
NES_SNSS::SaveSNSS(const char* fn, NES* nes)
{
  SNSS_FILE* snssFile = NULL;
  
  if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_WRITE))
  {
    ERROR("error opening SNSS file");
    goto error;
  }
  
  // write BASR
  if(!extract_BASR(&(snssFile->baseBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_BASR))
    {
      ERROR("error SNSS_BASR")
      goto error;
    }
  }

  // write VRAM
  if(!extract_VRAM(&(snssFile->vramBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_VRAM))
    {
      ERROR("error SNSS_VRAM");
      goto error;
    }
  }
  
  // write SRAM
  if(!extract_SRAM(&(snssFile->sramBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SRAM))
    {
      ERROR("error SNSS_SRAM");
      goto error;
    }
  }
  
  // write MPRD
  if(!extract_MPRD(&(snssFile->mapperBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_MPRD))
    {
      ERROR("error SNSS_MPRD");
      goto error;
    }
  }
  
  // write CNTR
  if(!extract_CNTR(&(snssFile->contBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_CNTR))
    {
      ERROR("error SNSS_CNTR");
      goto error;
    }
  }
  
  // write SOUN
  if(!extract_SOUN(&(snssFile->soundBlock), nes)) 
  {
    if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SOUN))
    {
      ERROR("error SNSS_SOUN");
      goto error;
    }
  }
  
  // write Extra Mapper Data
  extract_ExMPRD(snssFile, nes);
  
  if(SNSS_OK != SNSS_CloseFile(&snssFile))
  {
    ERROR("error closing SNSS file");
    goto error;
  }
  snssFile = NULL;
  
  return true;
  
error:
  if (snssFile) SNSS_CloseFile(&snssFile);
  return false;
}

