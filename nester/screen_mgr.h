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

#ifndef _SCREEN_MGR_H_
#define _SCREEN_MGR_H_

#include "types.h"

class screen_mgr
{
public:
  screen_mgr() {}
  virtual ~screen_mgr() {};

  virtual uint32 get_width() = 0;
  virtual uint32 get_height() = 0;

  virtual boolean set_palette(const uint8 pal[256][3]) = 0;
  virtual boolean get_palette(uint8 pal[256][3]) = 0;
  virtual boolean set_palette_section(uint8 start, uint8 len, const uint8 pal[][3]) = 0;
  virtual boolean get_palette_section(uint8 start, uint8 len, uint8 pal[][3]) = 0;

  virtual void assert_palette() = 0;

protected:
private:
};

#endif
