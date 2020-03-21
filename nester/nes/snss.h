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

#ifndef _SNSS_H_
#define _SNSS_H_

// SNSS-TFF: Standardized NES Save State Tagged File Format

#include "types.h"
#include "nes.h"
#include "libsnss.h"

class NES_SNSS
{
 public:
  boolean LoadSNSS(const char* fn, NES* nes);
  boolean SaveSNSS(const char* fn, NES* nes);
  
 private:
  void adopt_BASR(SnssBaseBlock* block, NES* nes);
  void adopt_VRAM(SnssVramBlock* block, NES* nes);
  void adopt_SRAM(SnssSramBlock* block, NES* nes);
  void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  void adopt_ExMPRD(SNSS_FILE *snssFile, NES* nes);
  void adopt_CNTR(SnssControllersBlock* block, NES* nes);
  void adopt_SOUN(SnssSoundBlock* block, NES* nes);
  
  int extract_BASR(SnssBaseBlock* block, NES* nes);
  int extract_VRAM(SnssVramBlock* block, NES* nes);
  int extract_SRAM(SnssSramBlock* block, NES* nes);
  int extract_MPRD(SnssMapperBlock* block, NES* nes);
  void extract_ExMPRD(SNSS_FILE *snssFile, NES* nes);
  int extract_CNTR(SnssControllersBlock* block, NES* nes);
  int extract_SOUN(SnssSoundBlock* block, NES* nes);
};


#endif
