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

#ifndef _NES_APU_WRAPPER_H_
#define _NES_APU_WRAPPER_H_

#include <stdio.h>
#include "types.h"
#include "sound_mgr.h"
#include "nes_apu.h"

#include "libsnss.h"

class NES;  // class prototype

class NES_APU
{
  // SNSS functions
  friend class NES_SNSS;

public:
  ~NES_APU();
  bool initialize (NES *parent);
  
  void reset();
  void snd_mgr_changed();

  uint8 Read(uint32 addr);
  void  Write(uint32 addr, uint8 data);

  uint8 ExRead(uint32 addr);
  void  ExWrite(uint32 addr, uint8 data);
  void  SelectExSound(nes_apu_exsound_type_t ex_type) { apu_set_exsound (ex_type); }

  void SyncAPURegister();
  boolean SyncDMCRegister(uint32 cpu_cycles);

  void DoFrame();

  void freeze();
  void thaw();
  
  void set_exsound_enable(bool exsound_enable) { apu_set_exsound_enable (exsound_enable); }

protected:
  NES *parent_NES;
  
  apu_t *apu;
  
  void ShutDown();
  
  uint8 regs[0x16];

  sound_mgr::sound_buf_pos currently_playing_half;

  // this function should be called by a state loading function
  // with an array of the apu registers
  // reg 0x14 is not used
  void load_regs(const uint8 new_regs[0x16]);

  // this function should be called by a state saving function
  // to fill an array of the apu registers
  // reg 0x14 is not used
  void NES_APU::get_regs(uint8 reg_array[0x16]);
  
private:
};

#endif


