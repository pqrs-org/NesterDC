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

#include <string.h>
#include <stdlib.h>
#include "nes_apu_wrapper.h"
#include "nes.h"
#include "sound_mgr.h"
#include "debug.h"


bool
NES_APU::initialize (NES *parent)
{
  int sample_rate;
  int sample_bits;
  
  parent_NES = parent;
  parent_NES->snd_mgr->clear_buffer();
  apu = NULL;
  
  if(parent_NES->snd_mgr->IsNull())
  {
    // kinda filthy.
    sample_rate = 11025;
    sample_bits = 8;
  }
  else
  {
    sample_rate = parent_NES->snd_mgr->get_sample_rate();
    sample_bits = parent_NES->snd_mgr->get_sample_bits();
  }
  
  apu = apu_create(sample_rate, 60, 0, sample_bits);
  if (!apu) return false;
  
  reset ();
  
  return true;
}


NES_APU::~NES_APU()
{
  ShutDown();
}

void NES_APU::ShutDown()
{
  if(apu)
  {
    apu_destroy(&apu);
    apu = NULL;
  }
}

void NES_APU::reset()
{
  ASSERT(apu);
  
  apu_reset();
  memset(regs, 0x00, sizeof(regs));
}

void NES_APU::snd_mgr_changed()
{
  int sample_rate;
  int sample_bits;
  
  ASSERT(apu);
  
  if(parent_NES->snd_mgr->IsNull())
  {
    // irresponsibly filthy.
    sample_rate = 11025;
    sample_bits = 8;
  }
  else
  {
    sample_rate = parent_NES->snd_mgr->get_sample_rate();
    sample_bits = parent_NES->snd_mgr->get_sample_bits();
  }
  
  apu_setparams(sample_rate, 60, sample_bits);
}

uint8 NES_APU::Read(uint32 addr)
{
  ASSERT(apu);
  
  return apu_read(addr);
}

void NES_APU::Write(uint32 addr, uint8 data)
{
  ASSERT(apu);
  
  regs[addr - 0x4000] = data;
  apu_write(addr, data);
  apu_write_cur(addr, data);
}

uint8 NES_APU::ExRead(uint32 addr)
{
  ASSERT(apu);
  
  return ex_read(addr);
}

void NES_APU::ExWrite(uint32 addr, uint8 data)
{
  ASSERT(apu);
  
  ex_write(addr, data);
}

void NES_APU::DoFrame()
{
  sound_mgr* snd_mgr = parent_NES->snd_mgr;
  sound_mgr::sound_buf_pos cur_ph;  // cur playing half
  sound_mgr::sound_buf_pos cur_nph; // cur not-playing half
  
  uint8* buf;
  uint32 buf_len;
  
  ASSERT(apu);
  
  if(snd_mgr->IsNull())
  {
    apu_process(NULL, 0);
  }
  else
  {
    cur_ph = snd_mgr->get_currently_playing_half();
    
    if(cur_ph == currently_playing_half) return;
    
    cur_nph = currently_playing_half;
    currently_playing_half = cur_ph;
    
    if(!snd_mgr->lock(cur_nph, (void**)&buf, &buf_len))
    {
      LOG("couldn't lock sound buffer\n");
      return;
    }
    
    apu_process(buf, buf_len/(snd_mgr->get_sample_bits()/8));
    
    snd_mgr->unlock();
  }
}

void NES_APU::freeze()
{
  parent_NES->snd_mgr->clear_buffer();
}

void NES_APU::thaw()
{
  currently_playing_half = parent_NES->snd_mgr->get_currently_playing_half();
}

void NES_APU::load_regs(const uint8 new_regs[0x16])
{
  int i;
  
  ASSERT(apu);
  
  for(i = 0; i < 0x16; i++)
  {
    // reg 0x14 not used
    if(i == 0x14) continue;

    // write the DMC regs directly
    if((i >= 0x10) && (i <= 0x13))
    {
      apu->apus.dmc.regs[i - 0x10] = new_regs[i];
    }
    else
    {
      apu_write(0x4000 + i, new_regs[i]);
      apu_write_cur(0x4000 + i, new_regs[i]);
    }
  }
}

void NES_APU::get_regs(uint8 reg_array[0x16])
{
  ASSERT(apu);
  
  // copy last written values
  memcpy(reg_array, regs, 0x16);
  
  // copy in the per-channel stored values
  regs[APU_WRA0-0x4000] = apu->apus.rectangle[0].regs[0];
  regs[APU_WRA1-0x4000] = apu->apus.rectangle[0].regs[1];
  regs[APU_WRA2-0x4000] = apu->apus.rectangle[0].regs[2];
  regs[APU_WRA3-0x4000] = apu->apus.rectangle[0].regs[3];
  regs[APU_WRB0-0x4000] = apu->apus.rectangle[1].regs[0];
  regs[APU_WRB1-0x4000] = apu->apus.rectangle[1].regs[1];
  regs[APU_WRB2-0x4000] = apu->apus.rectangle[1].regs[2];
  regs[APU_WRB3-0x4000] = apu->apus.rectangle[1].regs[3];
  regs[APU_WRC0-0x4000] = apu->apus.triangle.regs[0];
  regs[APU_WRC2-0x4000] = apu->apus.triangle.regs[1];
  regs[APU_WRC3-0x4000] = apu->apus.triangle.regs[2];
  regs[APU_WRD0-0x4000] = apu->apus.noise.regs[0];
  regs[APU_WRD2-0x4000] = apu->apus.noise.regs[1];
  regs[APU_WRD3-0x4000] = apu->apus.noise.regs[2];
  regs[APU_WRE0-0x4000] = apu->apus.dmc.regs[0];
  regs[APU_WRE1-0x4000] = apu->apus.dmc.regs[1];
  regs[APU_WRE2-0x4000] = apu->apus.dmc.regs[2];
  regs[APU_WRE3-0x4000] = apu->apus.dmc.regs[3];
}

void NES_APU::SyncAPURegister()
{
  sync_apu_register();
}

boolean NES_APU::SyncDMCRegister(uint32 cpu_cycles)
{
  return sync_dmc_register(cpu_cycles);
}

