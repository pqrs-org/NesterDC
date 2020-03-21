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
** changes for NesterDC:
** changes are marked with KAF
** 
** added #include libc.h, stdlib.h
** commented out #include <stdio.h>
** type casted NULL values to get them to compile 
**
** - Ken Friece
*/


#include "stdlib.h" //KAF
#include "nes_6502.h"
#include "nes.h"
#include "debug.h"

// NOT SAFE FOR MULTIPLE NES_6502'S
static NES *nes_6502_parent_NES = NULL;

void
NES_6502_NES_write(uint32 address, uint8 value)
{
  nes_6502_parent_NES->MemoryWrite(address, value);
}

uint8
NES_6502_NES_read(uint32 address)
{
  return nes_6502_parent_NES->MemoryRead(address);
}

bool
NES_6502::initialize (NES *parent)
{
  nes6502_context *context;
  
  if (!parent) return false;
  
  nes_6502_parent_NES = parent;
  
  context = nes6502_get_current_context ();
  context->read_handler = NES_6502_NES_read;
  context->write_handler = NES_6502_NES_write;
  return true;
}


