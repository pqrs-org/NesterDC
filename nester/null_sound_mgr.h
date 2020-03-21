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

#ifndef _NULL_SOUND_MGR_H_
#define _NULL_SOUND_MGR_H_

#include "sound_mgr.h"

class null_sound_mgr : public sound_mgr
{
public:
  void reset() {}
  
  // lock down for a period of inactivity
  void freeze() {}
  void thaw() {}
  
  void clear_buffer() {}
  
  boolean lock(sound_buf_pos which, void** buf, uint32* buf_len) { return FALSE; }
  void unlock() {}
  
  int get_sample_rate() { return 0; }
  int get_sample_bits() { return 0; }
  
  // returns SOUND_BUF_LOW or SOUND_BUF_HIGH
  sound_buf_pos get_currently_playing_half() { return SOUND_BUF_LOW; }
  
  boolean IsNull() { return TRUE; }
};

#endif
