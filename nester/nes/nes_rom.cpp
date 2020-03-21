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
** nes_rom.cpp.original
*/

#include "nester.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nes_rom.h"
#include "nes.h"
#include "debug.h"

bool
NES_ROM::initialize (const char *filename)
{
  mapper = 0;
  crc = 0;
  fds = 0;
  image_type = invalid_image;
  
  trainer    = NULL;
  ROM_banks  = NULL;
  VROM_banks = NULL;
  
  *rom_name = '\0';
  *rom_path = '\0';
  
  return load_rom (filename);
}


NES_ROM::~NES_ROM()
{
  if (trainer)     free (trainer);
  if (ROM_banks)   free (ROM_banks);
  if (VROM_banks)  free (VROM_banks);
}


uint8
NES_ROM::normalize_bank_num (uint8 bank_num)
{
  uint8 rv = 0x1;
  
  if (bank_num == 0) return 0;
  
  while (bank_num > rv)
    rv <<= 1;
  
  return rv;
}


bool
NES_ROM::load_rom (const char *filename)
{
  FILE *fp = NULL;
  
  set_path_info (filename);
  
  fp = fopen (filename, "r");
  if (!fp) goto error;
  
  if (fread ((void*)&header, 16, 1, fp) != 1) 
    goto error;
  
  /* patch for 260-in-1(#235) */
  header.num_16k_rom_banks = (!header.dummy) ? 256 : header.dummy;
  
  /* set num_banks to 2^x */
  header.num_16k_rom_banks = normalize_bank_num(header.num_16k_rom_banks);
  header.num_8k_vrom_banks = normalize_bank_num(header.num_8k_vrom_banks);
  
  if(!memcmp(header.id, "NES", 3) && (header.ctrl_z == 0x1A)) 
  {
    image_type = load_nes_rom (fp);
  }
  else if(!memcmp(header.id, "FDS", 3) && (header.ctrl_z == 0x1A))
  {
    image_type = load_fds_rom (fp);
  }
  else
    goto error;
  
  fclose (fp);
  
  if (image_type == invalid_image)
    goto error;
  
  correct_rom_image ();
  
  return true;
  
error:
  if (fp) fclose (fp);
  image_type = invalid_image;
  
  return false;
}


NES_ROM::rom_image_type
NES_ROM::load_nes_rom (FILE *fp)
{
  size_t nread;
  
  ROM_banks = (uint8*)malloc(header.num_16k_rom_banks * (16*1024));
  if (!ROM_banks) return invalid_image;
  
  VROM_banks = (uint8*)malloc(header.num_8k_vrom_banks * (8*1024));
  if (!VROM_banks) return invalid_image;
  
  if (has_trainer()) 
  {
    trainer = (uint8*)malloc(TRAINER_LEN);
    if(!trainer) return invalid_image;
    
    if (fread (trainer, TRAINER_LEN, 1, fp) != 1) 
      return invalid_image;
  }
  
  nread = 16 * 1024 * header.num_16k_rom_banks; 
  if (nread)
  {
    if (fread(ROM_banks, nread, 1, fp) != 1) 
      return invalid_image;
  }
  
  nread = 8 * 1024 * header.num_8k_vrom_banks;
  if (nread)
  {
    if (fread(VROM_banks, nread, 1, fp) != 1) 
      return invalid_image;
  }
  
  crc = calc_crc();
  
  return ines_image;
}


NES_ROM::rom_image_type
NES_ROM::load_fds_rom (FILE *fp)
{
  int i;
  uint8 disk_num = header.num_16k_rom_banks;
  
  header.id[0] = 'N';
  header.id[1] = 'E';
  header.id[2] = 'S';
  header.num_16k_rom_banks *= 4;
  header.num_8k_vrom_banks = 0;
  header.flags_1 = 0x40;
  header.flags_2 = 0x10;
  memset(header.reserved, 0, 8);
  
  ROM_banks = (uint8*)malloc(16 + 65500 * disk_num);
  if(!ROM_banks) return invalid_image;
  
  VROM_banks = (uint8*)malloc (1); /* dummy */
  if(!VROM_banks) return invalid_image;
  
  if (fread (ROM_banks + 16, 65500 * disk_num, 1, fp) != 1)
    return invalid_image;
  
  ROM_banks[0] = 'F';
  ROM_banks[1] = 'D';
  ROM_banks[2] = 'S';
  ROM_banks[3] = 0x1A;
  ROM_banks[4] = disk_num;
  
  mapper = 20;
  fds = (ROM_banks[0x1f] << 24) | (ROM_banks[0x20] << 16) |
  (ROM_banks[0x21] <<  8) | (ROM_banks[0x22] <<  0);
  
  for(i = 0; i < disk_num; i++)
  {
    int file_num = 0;
    uint32 pt = 16 + (65500 * i) + 0x3a;
    while(ROM_banks[pt] == 0x03)
    {
      pt += 0x0d;
      pt += ROM_banks[pt] + ROM_banks[pt + 1] * 256 + 4;
      file_num++;
    }
    ROM_banks[16 + (65500 * i) + 0x39] = file_num;
  }
  
  return fds_image;
}


void
NES_ROM::correct_rom_image ()
{
  bool header_valid = true;
  int i;
  
  mapper = (header.flags_1 >> 4);
  monitor_type = monitor_ntsc;
  
#include "nes_rom_correct.cpp"
  
  // if there is anything in the reserved bytes,
  // don't trust the high nybble of the mapper number
  for(i = 0; i < sizeof(header.reserved); ++i) 
  {
    if(header.reserved[i] != 0x00) 
    {
      header_valid = false;
      break;
    }
  }
  
  if (header_valid) 
    mapper |= (header.flags_2 & 0xF0);
  else
    LOG("Invalid NES header ($8-$F)");
}


uint32
NES_ROM::calc_crc ()
{
  uint32 i, j;
  uint32 c, crctable[256];
  uint32 nes_crc = 0;
  
  for(i = 0; i < 256; i++) {
    c = i;
    for (j = 0; j < 8; j++) {
      if (c & 1)
	c = (c >> 1) ^ 0xedb88320;
      else
	c >>= 1;
    }
    crctable[i] = c;
  }
  
  for(i = 0; i < header.num_16k_rom_banks; i++) {
    c = ~nes_crc;
    for(j = 0; j < 0x4000; j++)
      c = crctable[(c ^ ROM_banks[i*0x4000+j]) & 0xff] ^ (c >> 8);
    nes_crc = ~c;
  }
  
  return nes_crc;
}


const char* 
NES_ROM::GetRomCrc() 
{
  static char rom_crc[12];
  if (fds)
    sprintf(rom_crc, "F%x", fds);
  else
    sprintf(rom_crc, "%x", crc);
  return rom_crc;
}


void
NES_ROM::set_path_info(const char* fn)
{
  int basename = strlen(fn);
  const char *p;
  char *q;
  const char *r;
  
  while (basename)
  {
    if((fn[basename] == '\\') || (fn[basename] == '/'))
    {
      ++basename;
      break;
    }
    
    --basename;
  }
  
  p = fn + basename;
  q = rom_name;
  while (*p)
    *q++ = *p++;
  *q = '\0';
  
  p = fn;
  q = rom_path;
  r = fn + basename;
  while (p != r)
    *q++ = *p++;
  *q = '\0';
}


