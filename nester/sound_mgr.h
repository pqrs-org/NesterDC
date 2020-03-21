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

#ifndef _SOUND_MGR_H_
#define _SOUND_MGR_H_

#include "types.h"

class sound_mgr
{
public:
  virtual ~sound_mgr () {}
  // these values are used to indicate positions within the sound buffer
  enum sound_buf_pos
  {
    SOUND_BUF_LOW,
    SOUND_BUF_HIGH
  };
  
  virtual bool initialize () { return true; }
  
  virtual void reset() = 0;
  
  // lock down for a period of inactivity
  virtual void freeze() = 0;
  virtual void thaw() = 0;
  
  virtual void clear_buffer() = 0; // clears out sound buffer with silence
  
  virtual boolean lock(sound_buf_pos which, void** buf, uint32* buf_len) = 0;
  virtual void unlock() = 0;
  
  virtual int get_sample_rate() = 0;
  virtual int get_sample_bits() = 0;
  
  // returns SOUND_BUF_LOW or SOUND_BUF_HIGH
  virtual sound_buf_pos get_currently_playing_half() = 0;
  
  virtual boolean IsNull() = 0;
private:
protected:
};

#endif
