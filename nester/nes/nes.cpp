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

/* 
** NesterDC by Ken Friece
** This routine was so different than the original, that I included 
** diff output at the bottom of the routine. The orginal is also called
** nes.cpp.original
*/

#include "nester.h"
#include "stdlib.h"
#include <string.h>
#include <math.h>
#include "nes.h"
#include "nes_mapper.h"
#include "nes_rom.h"
#include "nes_ppu.h"
#include "snss.h"
#include "debug.h"

/* 
   const float NES::CYCLES_PER_LINE = (float)113.6;
   const float NES::CYCLES_PER_LINE = (float)113.852;
   const float NES::CYCLES_PER_LINE = (float)113.75;
   const float NES::CYCLES_PER_LINE = (float)113.6666666666666666666;
*/


NES::NES(sound_mgr* _sound_mgr)
{
  snd_mgr = _sound_mgr;
  
  LOG("nester - NES emulator by Darren Ranalli, (c) 2000");
  
  cpu = NULL;
  ppu = NULL;
  apu = NULL;
  ROM = NULL;
  mapper = NULL;
  
  DPCM_IRQ = true;
  is_frozen = FALSE;
  
  nes_nnnesterj_cheatinfo_size = 0;
  
  memset(SaveRAM, 0, sizeof(SaveRAM));
}


bool 
NES::initialize (const char *filename)
{
  LOG("Creating NES CPU");
  cpu = new NES_6502 ();
  if (!cpu) return false;
  if (!(cpu->initialize(this))) return false;
  
  LOG("Creating NES PPU");
  ppu = new NES_PPU ();
  if (!ppu) return false;
  if (!(ppu->initialize(this))) return false;
  
  LOG("Creating NES APU");
  apu = new NES_APU ();
  if (!apu) return false;
  if (!(apu->initialize(this))) return false;
  
  LOG("Creating SNSS");
  snss = new NES_SNSS ();
  if (!snss) return false;
  
  if (!(loadROM(filename))) return false;
  
  clear_genie ();
  
  return true;
}


NES::~NES()
{
  freeROM();
  
  if(cpu) delete cpu;
  if(ppu) delete ppu;
  if(apu) delete apu;
  if(snss) delete snss;
}

void NES::new_snd_mgr(sound_mgr* _sound_mgr)
{
  snd_mgr = _sound_mgr;
  apu->snd_mgr_changed();
}

bool
NES::loadROM(const char* fn)
{
  LOG("Loading ROM...");
  ROM = new NES_ROM ();
  if (!ROM) goto error;
  if (!(ROM->initialize (fn))) goto error;
  
  LOG("Set up the mapper");
  mapper = GetMapper(ROM->get_mapper_num());
  if (!mapper) goto error;
  if (!(mapper->initialize (this, ppu))) goto error;
  
  reset();
  
  LOG("Starting emulation...");
  
  return true;
  
error:
  freeROM ();
  return false;
}

void NES::freeROM()
{
  if(ROM)
  {
    delete ROM;
    ROM = NULL;
  }
  if(mapper)
  {
    delete mapper;
    mapper = NULL;
  }
}

const char* 
NES::getROMname()
{
  return ROM->GetRomName();
}

const char* 
NES::getROMpath()
{
  return ROM->GetRomPath();
}


void
NES::reset()
{
  // clear RAM
  memset(RAM, 0x00, sizeof(RAM));

  softreset();
}


void
NES::softreset()
{
  int i;
  
  // set up CPU
  {
    nes6502_context *context;
    memset((void*)&context, 0x00, sizeof(context));
    
    context = nes6502_get_current_context ();
    context->mem_page[0] = RAM;
    context->mem_page[3] = SaveRAM;
    nes6502_setcontext();
    nes6502_set_zeropage();
  }
  
  // reset the PPU
  ppu->reset();
  ppu->vram_write_protect = ROM->get_num_8k_VROM_banks() ? 1 : 0;
  
  // reset the APU
  apu->reset();
  
  frame_irq_enabled = 0xFF;
  frame_irq_disenabled = 0;
  
  if(mapper)
  {
    // reset the mapper
    mapper->Reset();
  }
  
  // reset the CPU
  cpu->Reset();
  
  // set up the trainer if present
  if(ROM->has_trainer())
  {
    // trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000
    memcpy(&SaveRAM[0x1000], ROM->get_trainer(), NES_ROM::TRAINER_LEN);
  }
  
#if 0
#include "NES_set_VSPalette.cpp"
#endif
#include "nes_set_controller.cpp"
#include "nes_set_cycles.cpp"
  
  /* reset external devices */
  NES_gun.initialize();
  NES_kbd.initialize();
  NES_hypershot.initialize();
  NES_paddle.initialize();
  NES_mahjong.initialize();
  
  ideal_cycle_count  = 0.0;
  emulated_cycle_count = 0;
  
  disk_side_flag = 0;
  
  pad_strobe = FALSE;
  for (i = 0; i < 4; ++i)
    pad_bits[i] = 0;
}


#define EMULATE_FRAME
#  include "nes_emulate_frame.cpp"
#undef EMULATE_FRAME
#  include "nes_emulate_frame.cpp"


void
NES::freeze()
{
  if (!is_frozen) 
  {
    apu->freeze();
    is_frozen = TRUE;
  }
}

void
NES::thaw()
{
  if (is_frozen) 
  {
    apu->thaw();
    is_frozen = FALSE;
  }
}

boolean
NES::frozen()
{
  return is_frozen;
}

uint8
NES::MemoryRead(uint32 addr)
{
//  LOG("Read " << HEX(addr,4) << endl);

  if(addr < 0x2000) // RAM
  {
    return ReadRAM(addr);
  }
  else if(addr < 0x4000) // low registers
  {
    return ReadLowRegs(addr);
  }
  else if(addr < 0x4018) // high registers
  {
    return ReadHighRegs(addr);
  }
  else if(addr < 0x6000) // mapper low
  {
//    LOG("MAPPER LOW READ: " << HEX(addr,4) << endl);
//    return((uint8)(addr >> 8)); // what's good for conte is good for me
    return mapper->MemoryReadLow(addr);
  }
  else // save RAM, or ROM (mapper 40)
  {
    mapper->MemoryReadSaveRAM(addr);
    return cpu->GetByte(addr);
  }
}

void
NES::MemoryWrite(uint32 addr, uint8 data)
{
//  LOG("Write " << HEX(addr,4) << " " << HEX(data,2) << endl);

  if(addr < 0x2000) // RAM
  {
    WriteRAM(addr, data);
  }
  else if(addr < 0x4000) // low registers
  {
    WriteLowRegs(addr, data);
  }
  else if(addr < 0x4018) // high registers
  {
    WriteHighRegs(addr, data);
    mapper->WriteHighRegs(addr, data);
  }
  else if(addr < 0x6000) // mapper low
  {
    mapper->MemoryWriteLow(addr, data);
  }
  else if(addr < 0x8000) // save RAM
  {
    SaveRAM[addr - 0x6000] = data;
    mapper->MemoryWriteSaveRAM(addr, data);
  }
  else // mapper
  {
    mapper->MemoryWrite(addr, data);
  }
}


uint8
NES::ReadRAM(uint32 addr)
{
  return RAM[addr & 0x7FF];
}

void
NES::WriteRAM(uint32 addr, uint8 data)
{
  RAM[addr & 0x7FF] = data;
}


uint8
NES::ReadLowRegs(uint32 addr)
{
  return ppu->ReadLowRegs(addr & 0xE007);
}

void
NES::WriteLowRegs(uint32 addr, uint8 data)
{
  ppu->WriteLowRegs(addr & 0xE007, data);
}


uint8
NES::ReadHighRegs(uint32 addr)
{
  if (addr == 0x4014) // SPR-RAM DMA
  {
    LOG("Read from SPR-RAM DMA reg??");
    return ppu->Read0x4014();
  } 
  else if ((addr == 0x4015) & !(frame_irq_enabled & 0xC0)) // frame_IRQ
  {
    return apu->Read(0x4015) | 0x40;
  }
  else if(addr < 0x4016) // APU
  {
//    LOG("APU READ:" << HEX(addr,4) << endl);
    return apu->Read(addr);
  }
  else // joypad regs
  {
    uint8 retval;
    
    if(addr == 0x4016)
    {
      // joypad 1
      retval = ((pad_bits[2] & 0x01) << 1) | (pad_bits[0] & 0x01);
      pad_bits[0] >>= 1;
      pad_bits[2] >>= 1;
      retval |= mic.ReadReg4016 ();
      
      if (ex_controller_type != EX_NONE)
      {
	switch (ex_controller_type) 
	{
	  case EX_FAMILY_KEYBOARD:
	    retval |= NES_kbd.ReadReg4016();
	    break;
	  case EX_SPACE_SHADOW_GUN:
	    retval |= NES_gun.ReadReg4016();
	    break;
	  case EX_ARKANOID_PADDLE:
	    retval |= NES_paddle.ReadReg4016();
	    break;
	}
      }
    }
    else
    {
      // joypad 2
      retval = ((pad_bits[3] & 0x01) << 1) | (pad_bits[1] & 0x01);
      pad_bits[1] >>= 1;
      pad_bits[3] >>= 1;
      
      if (ex_controller_type != EX_NONE) 
      {
        switch (ex_controller_type) 
        {
          case EX_FAMILY_KEYBOARD:
            retval |= NES_kbd.ReadReg4017();
            break;
          case EX_OPTICAL_GUN:
          case EX_SPACE_SHADOW_GUN:
            retval |= NES_gun.ReadReg4017(this);
            break;
          case EX_HYPER_SHOT:
            retval |= NES_hypershot.ReadReg4017();
            break;
	  case EX_ARKANOID_PADDLE:
	    retval |= NES_paddle.ReadReg4017();
	    break;
	  case EX_MAHJONG:
	    retval |= NES_mahjong.ReadReg4017();
	    break;
        }
      }
    }
    return retval;
  }
}

void
NES::WriteHighRegs(uint32 addr, uint8 data)
{
  if (addr == 0x4014) // SPR-RAM DMA
  {
    ppu->Write0x4014(data);
    cpu->SetDMA(514);
  }
  else if (addr < 0x4016) // APU
  {
    apu->Write(addr, data);
  }
  else if (addr == 0x4016) // joy pad
  {
    // bit 0 == joypad strobe
    if(data & 0x01)
    {
      pad_strobe = TRUE;
    }
    else
    {
      if(pad_strobe)
      {
	uint8 state;
        pad_strobe = FALSE;
        
        state = pad[2].get_state();
        pad_bits[0] = (1 << 19) | (state << 8) | pad[0].get_state ();
        pad_bits[2] = state;
	
        state = pad[3].get_state ();
        pad_bits[1] = (1 << 18) | (state << 8) | pad[1].get_state ();
        pad_bits[3] = state;
        mic.WriteReg4016 (this);
	
	if (ex_controller_type != EX_NONE)
	{
	  switch (ex_controller_type)
	  {
            case EX_CRAZY_CLIMBER:
              NES_cclimber.WriteReg4016(this);
              break;
	    case EX_SPACE_SHADOW_GUN:
	      NES_gun.WriteReg4016(this);
	      break;
	    case EX_HYPER_SHOT:
	      NES_hypershot.WriteReg4016(this);
	      break;
	    case EX_ARKANOID_PADDLE:
	      NES_paddle.WriteReg4016_strobe(this);
	      break;
	  }
	}
      }
    }
    
    if (ex_controller_type != EX_NONE)
    {
      switch (ex_controller_type)
      {
        case EX_FAMILY_KEYBOARD:
          NES_kbd.WriteReg4016(data);
          break;
	case EX_ARKANOID_PADDLE:
	  NES_paddle.WriteReg4016();
	  break;
	case EX_MAHJONG:
	  NES_mahjong.WriteReg4016(data);
	  break;
      }
    }
  }
  else if (addr == 0x4017) // frame-IRQ
  {
    if(!frame_irq_disenabled)
      frame_irq_enabled = data;
    apu->Write(addr, data);
  }
}

void 
NES::emulate_CPU_cycles(float num_cycles)
{
  uint32 cycle_deficit;
  
  ideal_cycle_count += num_cycles;
  cycle_deficit = ((uint32)ideal_cycle_count) - emulated_cycle_count;
  if(cycle_deficit > 0)
  {
    emulated_cycle_count += cpu->Execute(cycle_deficit);
    if(apu->SyncDMCRegister(cycle_deficit) && DPCM_IRQ)
    {
      cpu->DoPendingIRQ();
    }
  }
}

// call every once in a while to avoid cycle count overflow
void 
NES::trim_cycle_counts()
{
  uint32 trim_amount;
  
  trim_amount = (uint32)floor(ideal_cycle_count);
  if(trim_amount > emulated_cycle_count) trim_amount = emulated_cycle_count;
  
  ideal_cycle_count  -= (float)trim_amount;
  emulated_cycle_count -= trim_amount;
}


void
NES::set_SaveRAM (const uint8 *buffer, int size)
{
  memcpy(SaveRAM, buffer, size); 
}


bool
NES::is_invalid_sram ()
{
  int i;
  
  for (i = 0; i < sizeof(SaveRAM); ++i)
  {
    if (SaveRAM[i])
      return false;
  }
  return true;
}


void
NES::make_savedata (uint8 *buf)
{
  uint8 *rom_banks = ROM->get_ROM_banks ();
  uint8 *data_buf, *flag_buf;
  int n, i, j;
  
  memset(buf, 0, fds_save_buflen);
  memcpy(buf, "FDS2", 4);
  buf[4] = mapper->GetDiskSideNum();
  
  data_buf = buf + 16;
  flag_buf = buf + 16 + (4 * 65500);
  n = 0;
  for(i = 0; i < mapper->GetDiskSideNum(); ++i)
  {
    for(j = 0; j < 65500; ++j)
    {
      uint8 data = mapper->GetDiskData(i*0x10000+j);
      
      if (data != rom_banks[16 + 65500 * i + j])
      {
	data_buf[n] = data;
	flag_buf[n] = 1;
      }
      ++n;
    }
  }
}


void
NES::restore_savedata (uint8 *buf)
{
  uint8 *rom_banks = ROM->get_ROM_banks ();
  uint8 *data_buf, *flag_buf;
  int i, j, n;
  
  data_buf = buf + 16;
  flag_buf = buf + 16 + (4 * 65500);
  n = 0;
  for (i = 0; i < buf[4]; ++i)
  {
    for (j = 0; j < 65500; ++j)
    {
      uint8 data;
      
      if (flag_buf[n])
	data = data_buf[n];
      else
	data = rom_banks[16 + 65500 * i + j];
      mapper->SetDiskData(i*0x10000+j, data);
      
      ++n;
    }
  }
}


/* ============================================================ */
void
NES::load_genie (const char *genie)
{
  int i;
  char code[9];
  int len;
  uint32 gcode = 0;
  
  memcpy(code, genie, 9); 
  code[8] = '\0';
  
  len = 0;
  for(i = 0; i < 9; ++i)
  {
    switch(code[i])
    {
      case 'A': code[i] = 0x00; break;
      case 'P': code[i] = 0x01; break;
      case 'Z': code[i] = 0x02; break;
      case 'L': code[i] = 0x03; break;
      case 'G': code[i] = 0x04; break;
      case 'I': code[i] = 0x05; break;
      case 'T': code[i] = 0x06; break;
      case 'Y': code[i] = 0x07; break;
      case 'E': code[i] = 0x08; break;
      case 'O': code[i] = 0x09; break;
      case 'X': code[i] = 0x0A; break;
      case 'U': code[i] = 0x0B; break;
      case 'K': code[i] = 0x0C; break;
      case 'S': code[i] = 0x0D; break;
      case 'V': code[i] = 0x0E; break;
      case 'N': code[i] = 0x0F; break;
      default:  len = i; i = 9; break;
    }
  }

  if(len == 6)
  {
    uint32 addr = 0x0000,data = 0x0000;
    // address
    addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
    addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
    addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
    addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
    addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
    addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
    addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
    addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
    addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
    addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
    addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
    addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
    addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
    addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
    addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
    // value
    data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
    data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
    data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
    data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
    data |= (code[5] & 0x8) ? 0x0008 : 0x0000;
    data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
    data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
    data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
    gcode = (addr << 16) | data;
  }
  else if(len == 8)
  {
    uint32 addr = 0x0000,data = 0x0000;
    // address
    addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
    addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
    addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
    addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
    addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
    addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
    addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
    addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
    addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
    addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
    addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
    addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
    addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
    addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
    addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
    // value
    data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
    data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
    data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
    data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
    data |= (code[7] & 0x8) ? 0x0008 : 0x0000;
    data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
    data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
    data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
    // compare value
    data |= (code[6] & 0x8) ? 0x8000 : 0x0000;
    data |= (code[7] & 0x4) ? 0x4000 : 0x0000;
    data |= (code[7] & 0x2) ? 0x2000 : 0x0000;
    data |= (code[7] & 0x1) ? 0x1000 : 0x0000;
    data |= (code[5] & 0x8) ? 0x0800 : 0x0000;
    data |= (code[6] & 0x4) ? 0x0400 : 0x0000;
    data |= (code[6] & 0x2) ? 0x0200 : 0x0000;
    data |= (code[6] & 0x1) ? 0x0100 : 0x0000;
    gcode = (addr << 16) | data | 0x80000000;
  }
  else
    return;
  
  for (i = 0; i< genie_num; ++i)
  {
    if (genie_code[i] == gcode)
      break;
  }
  if (i == genie_num)
    genie_code[genie_num++] = gcode;
}


void NES::set_genie()
{
  int i, j;
  int num_16k_ROM_banks = ROM->get_num_16k_ROM_banks ();
  
  for (i = 0; i < genie_num; ++i)
  {
    uint32 code = genie_code[i]; 
    uint32 addr = ((code & 0x7FFF0000) >> 16) | 0x8000;
    uint8 data1 = code & 0x000000FF;
    uint8 data2 = (code & 0x0000FF00) >> 8;
    
    /* scan all CPU_banks */
    for (j = 0; j < num_16k_ROM_banks * 2; ++j)
    {
      uint8 *mem_page = ROM->get_ROM_banks () + (j << 13);
      uint8 data3 = mem_page[addr & 0x1FFF];
      
      if(!(code & 0x80000000) || data2 == data3)
      {
	mem_page[addr & 0x1FFF] = data1;
      }
    }
  }
}


/* ============================================================ */
bool
NES::loadState(const char* fn)
{
  bool success = snss->LoadSNSS(fn, this);
  if (success)
    ppu->set_palette ();
  return success;
}


bool
NES::saveState(const char* fn)
{
  return snss->SaveSNSS(fn, this);
}


uint8 NES::GetDiskData(uint32 pt)
{
  return mapper->GetDiskData(pt);
}
uint8 NES::GetDiskSideNum()
{
  return mapper->GetDiskSideNum();
}
uint8 NES::GetDiskSide()
{
  return mapper->GetDiskSide();
}
void NES::SetDiskSide(uint8 side)
{
  disk_side_flag = side | 0x10;
  mapper->SetDiskSide (disk_side_flag & 0x0f);
  disk_side_flag = 0;
}
uint8 NES::DiskAccessed()
{
  return mapper->DiskAccessed();
}


/* ============================================================ */
void
NES::load_nnnesterj_cheat (const char *filename)
{
  nes_nnnesterj_cheatinfo_size = NES_NNNESTERJ_CHEATINFO_SIZE;
  nnnesterj_cheat_load (nes_nnnesterj_cheatinfo, &nes_nnnesterj_cheatinfo_size, 
                        RAM, SaveRAM, filename);
  
  set_nnnesterj_cheat_alive_num ();
}


void
NES::set_nnnesterj_cheat_alive_num ()
{
  int i;
  
  nes_nnnesterj_cheatinfo_alive_num = 0;
  for (i = 0; i < nes_nnnesterj_cheatinfo_size; ++i)
  {
    if (nes_nnnesterj_cheatinfo[i].cheatinfo.check_flag)
      ++nes_nnnesterj_cheatinfo_alive_num;
  }
}


void
NES::apply_nnnesterj_cheat()
{
  int i;
  nes_nnnesterj_cheatinfo_t *nnc;
  nnnesterj_cheatinfo_t *p;
  
  nnc = nes_nnnesterj_cheatinfo;
  for (i = 0; i < nes_nnnesterj_cheatinfo_size; ++i) 
  {
    p = &(nnc->cheatinfo);
    
    if (p->check_flag) 
    {
      if (p->size == 0) 
      {
        nnc->mem_base[p->address] = p->data;
      }
      else if (p->size == 1)
      {
        nnc->mem_base[p->address] = p->data;
        nnc->mem_base[(p->address + 1) & nnc->mem_mask] = p->data >> 8;
      }
      else if (p->size == 2)
      {
        /* NNNesterJ cheat code bug?? */
        /* if p->size == 2 then realsize == 4. */
        nnc->mem_base[p->address] = p->data;
        nnc->mem_base[(p->address + 1) & nnc->mem_mask] = p->data >> 8;
        nnc->mem_base[(p->address + 2) & nnc->mem_mask] = p->data >> 16;
        nnc->mem_base[(p->address + 3) & nnc->mem_mask] = p->data >> 24;
      }
      else
      {
        nnc->mem_base[p->address] = p->data;
        nnc->mem_base[(p->address + 1) & nnc->mem_mask] = p->data >> 8;
        nnc->mem_base[(p->address + 2) & nnc->mem_mask] = p->data >> 16;
      }
    }
    
    ++nnc;
  }
}  


