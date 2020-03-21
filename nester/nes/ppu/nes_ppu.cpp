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
*/

/*
** NesterDC and NesterDC-SDL by Takayama Fumihiko. 
*/

#include <stdlib.h>
#include <string.h> 
#include "nes_ppu.h"
#include "nes.h"
#include "nes_mapper.h"

#include "debug.h"

static const uint16 rgb565_pal[64] = {
#include "rgb565_pal.h"
};

#define VRAM(addr) \
  PPU_VRAM_banks[(addr) >> 10][(addr) & 0x3FF]

/*
scanline start (if background or sprites are enabled):
	v:0000010000011111=t:0000010000011111
*/
#define LOOPY_SCANLINE_START(v,t) \
  { \
    v = (v & 0xFBE0) | (t & 0x041F); \
  }

/*
bits 12-14 are the tile Y offset.
you can think of bits 5,6,7,8,9 as the "y scroll"(*8).  this functions
slightly different from the X.  it wraps to 0 and bit 11 is switched when
it's incremented from _29_ instead of 31.  there are some odd side effects
from this.. if you manually set the value above 29 (from either 2005 or
2006), the wrapping from 29 obviously won't happen, and attrib data will be
used as name table data.  the "y scroll" still wraps to 0 from 31, but
without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
appeared as a negative scroll value.
*/
#define LOOPY_NEXT_LINE(v) \
  { \
    if((v & 0x7000) == 0x7000) /* is subtile y offset == 7? */ \
    { \
      v &= 0x8FFF; /* subtile y offset = 0 */ \
      if((v & 0x03E0) == 0x03A0) /* name_tab line == 29? */ \
      { \
        v ^= 0x0800;  /* switch nametables (bit 11) */ \
        v &= 0xFC1F;  /* name_tab line = 0 */ \
      } \
      else \
      { \
        if((v & 0x03E0) == 0x03E0) /* line == 31? */ \
        { \
          v &= 0xFC1F;  /* name_tab line = 0 */ \
        } \
        else \
        { \
          v += 0x0020; \
        } \
      } \
    } \
    else \
    { \
      v += 0x1000; /* next subtile y offset */ \
    } \
  }

#define CHECK_MMC2(addr) \
  if(((addr) & 0x0FC0) == 0x0FC0) \
  { \
    if((((addr) & 0x0FF0) == 0x0FD0) || (((addr) & 0x0FF0) == 0x0FE0)) \
    { \
      parent_NES->mapper->PPU_Latch_FDFE(addr); \
    } \
  }


bool
NES_PPU::initialize(NES* parent)
{
  parent_NES = parent;
  return true;
}

void NES_PPU::reset()
{
  // reset registers
  memset(LowRegs, 0x00, sizeof(LowRegs));
  HighReg0x4014 = 0x00;

  // clear sprite RAM
  memset(spr_ram, 0x00, sizeof(spr_ram));

  // clear palettes
  memset(bg_pal,  0x00, sizeof(bg_pal));
  memset(spr_pal, 0x00, sizeof(spr_pal));
  memset(bg_pal_color, 0, sizeof(bg_pal_color));
  memset(spr_pal_color, 0, sizeof(spr_pal_color));
  
  // clear solid buffer
  memset(solid_buf, 0x00, sizeof(solid_buf));
  
  // clear pattern tables
  memset(PPU_patterntables, 0x00, sizeof(PPU_patterntables));

  // clear internal name tables
  memset(PPU_nametables, 0x00, sizeof(PPU_nametables));

  // clear VRAM page table
  memset(PPU_VRAM_banks, 0x00, sizeof(PPU_VRAM_banks));

  memset(spr_per_line, 0, sizeof(spr_per_line));
  
  // set up PPU memory space table
  PPU_VRAM_banks[0x00] = PPU_patterntables + (0*0x400);
  PPU_VRAM_banks[0x01] = PPU_patterntables + (1*0x400);
  PPU_VRAM_banks[0x02] = PPU_patterntables + (2*0x400);
  PPU_VRAM_banks[0x03] = PPU_patterntables + (3*0x400);

  PPU_VRAM_banks[0x04] = PPU_patterntables + (4*0x400);
  PPU_VRAM_banks[0x05] = PPU_patterntables + (5*0x400);
  PPU_VRAM_banks[0x06] = PPU_patterntables + (6*0x400);
  PPU_VRAM_banks[0x07] = PPU_patterntables + (7*0x400);

  // point nametables at internal name table 0
  PPU_VRAM_banks[0x08] = PPU_nametables;
  PPU_VRAM_banks[0x09] = PPU_nametables;
  PPU_VRAM_banks[0x0A] = PPU_nametables;
  PPU_VRAM_banks[0x0B] = PPU_nametables;
  
  read_2007_buffer = 0x00;
  in_vblank = 0;
  bg_pattern_table_bank_base = 0;
  spr_pattern_table_bank_base = 0;
  ppu_addr_inc = 0;
  loopy_v = 0;
  loopy_t = 0;
  loopy_x = 0;
  toggle_2005_2006 = 0;
  spr_ram_rw_ptr = 0;
  read_2007_buffer = 0;
  current_frame_line = 240;
  
  // set mirroring
  set_mirroring(parent_NES->ROM->get_mirroring());
}

void NES_PPU::set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3)
{
  ASSERT(nt0 < 4); ASSERT(nt1 < 4); ASSERT(nt2 < 4); ASSERT(nt3 < 4);
  PPU_VRAM_banks[0x08] = PPU_nametables + (nt0 << 10); // * 0x0400
  PPU_VRAM_banks[0x09] = PPU_nametables + (nt1 << 10);
  PPU_VRAM_banks[0x0A] = PPU_nametables + (nt2 << 10);
  PPU_VRAM_banks[0x0B] = PPU_nametables + (nt3 << 10);
}

void NES_PPU::set_mirroring(mirroring_type m)
{
  if(MIRROR_FOUR_SCREEN == m)
  {
    set_mirroring(0,1,2,3);
  }
  else if(MIRROR_HORIZ == m)
  {
    set_mirroring(0,0,1,1);
  }
  else if(MIRROR_VERT == m)
  {
    set_mirroring(0,1,0,1);
  }
  else
  {
    LOG("Invalid mirroring type\n");
    set_mirroring(MIRROR_FOUR_SCREEN);
  }
}

/* need for SNSS */
void 
NES_PPU::set_palette ()
{
  int i;
  
  for (i = 0; i < 0x10; ++i)
  {
    bg_pal_color[i] = rgb565_pal[bg_pal[i]];
    spr_pal_color[i] = rgb565_pal[spr_pal[i]];
  }
  
  /* spr_pal_color[0,4,8,12] don't used, and must be 0 (for render_spr) */
  spr_pal_color[0] = 0;
  spr_pal_color[4] = 0;
  spr_pal_color[8] = 0;
  spr_pal_color[12] = 0;
}

void NES_PPU::start_frame()
{
  current_frame_line = 0;

  if(bg_or_spr_enabled ())
  {
    loopy_v = loopy_t;
  }
  
  {
    /* build all sprite position */
    sprites_per_line *p;
    uint8 *spr;
    int i;
    
    p = spr_per_line;
    for (i = 0; i < (256 + 32) / 8; ++i)
    {
      p->num = 0; ++p;
      p->num = 0; ++p;
      p->num = 0; ++p;
      p->num = 0; ++p;
      
      p->num = 0; ++p;
      p->num = 0; ++p;
      p->num = 0; ++p;
      p->num = 0; ++p;
    }

#define BUILD_SPR_8() \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \
 \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \
    (p->sprite_index)[(p->num)++] = i; ++p; \

    
    if (sprite_height () == 8)
    {
      /*
	DON'T reverse sort on spr_per_line 
	because sprites must be rendered by #0 for MMC2 
      */
      for (i = 1, spr = spr_ram + 4;
	   i < 64;
	   ++i, spr += 4)
      {
	p = spr_per_line + (*spr + 1);
	
	BUILD_SPR_8();
      }
    }
    else
    {
      for (i = 1, spr = spr_ram + 4;
	   i < 64;
	   ++i, spr += 4)
      {
	p = spr_per_line + (*spr + 1);
	
	BUILD_SPR_8();
	BUILD_SPR_8();
      }
    }
  }
}


#define DO_SCANLINE_BANKSWITCH_PER_TILE
#include "do_scanline.cpp"
#undef DO_SCANLINE_BANKSWITCH_PER_TILE

#define DO_SCANLINE_MMC5
#include "do_scanline.cpp"
#undef DO_SCANLINE_MMC5

#define DO_SCANLINE_MMC2
#include "do_scanline.cpp"
#undef DO_SCANLINE_MMC2

#include "do_scanline.cpp"


void NES_PPU::end_frame()
{
}

void NES_PPU::start_vblank()
{
  in_vblank = 1;

  // set vblank register flag
  LowRegs[2] |= 0x80;
}

void NES_PPU::end_vblank()
{
  in_vblank = 0;

  // reset vblank register flag and sprite0 hit flag1
  LowRegs[2] &= 0x3F;
}


// these functions read from/write to VRAM using loopy_v
uint8 NES_PPU::read_2007()
{
  uint16 addr;
  uint8 temp;
  
  addr = loopy_v;
  loopy_v += ppu_addr_inc;

  ASSERT(addr < 0x4000);
  addr &= 0x3FFF;

  if(addr >= 0x3000)
  {
    // is it a palette entry?
    if(addr >= 0x3F00)
    {
      // palette

      // handle palette mirroring
      if(0x0000 == (addr & 0x0010))
      {
        // background palette
        return bg_pal[addr & 0x000F];
      }
      else
      {
        // sprite palette
        return spr_pal[addr & 0x000F];
      }
    }

    // handle mirroring
    addr &= 0xEFFF;
  }

  temp = read_2007_buffer;
  read_2007_buffer = VRAM(addr);

  return temp;
}

void NES_PPU::write_2007(uint8 data)
{
  uint16 addr;

  addr = loopy_v;
  loopy_v += ppu_addr_inc;

  addr &= 0x3FFF;

//  LOG("PPU 2007 WRITE: " << HEX(addr,4) << " " << HEX(data,2) << endl);

  if(addr >= 0x3000)
  {
    // is it a palette entry?
    if(addr >= 0x3F00)
    {
      // palette
      data &= 0x3F;
      
      if(0x0000 == (addr & 0x000f)) 
      {
	/* color #0 */
        bg_pal[0] = 
	  bg_pal[4] = 
	  bg_pal[8] = 
	  bg_pal[12] = 
          spr_pal[0] = 
          spr_pal[4] = 
          spr_pal[8] = 
          spr_pal[12] = 
	  data;
        
        /* spr_pal_color[0,4,8,12] don't used, and must be 0 (for render_spr) */
	bg_pal_color[0] = 
	  bg_pal_color[4] =
	  bg_pal_color[8] = 
	  bg_pal_color[12] = 
	  rgb565_pal[data];
      }
      else if(0x0000 == (addr & 0x0010))
      {
	/* background palette (writing 0, 4, 8, 12 must ignore) */
	if (addr & 0x03)
	{
	  bg_pal[addr & 0x000F] = data;
	  bg_pal_color[addr & 0x000F] = rgb565_pal[data];
	}
      }
      else
      {
        /* sprite palette (writing 0, 4, 8, 12 must ignore) */
	if (addr & 0x03)
	{
	  spr_pal[addr & 0x000F] = data;
	  spr_pal_color[addr & 0x000F] = rgb565_pal[data];
	}
      }
      
      return;
    }
    
    // handle mirroring
    addr &= 0xEFFF;
  }

  if (!(vram_write_protect && addr < 0x2000))
    VRAM(addr) = data;
}


uint8 NES_PPU::ReadLowRegs(uint32 addr)
{
  ASSERT((addr >= 0x2000) && (addr < 0x2008));

//  LOG("PPU Read " << HEX(addr,4) << endl);

  switch(addr)
  {
    case 0x2002:
      {
        uint8 temp;

        // clear toggle
        toggle_2005_2006 = 0;

        temp = LowRegs[2];

        // clear v-blank flag
        LowRegs[2] &= 0x7F;

        return temp;
      }
      break;

    case 0x2007:
      return read_2007();
      break;

  }

  return LowRegs[addr & 0x0007];
}

void  NES_PPU::WriteLowRegs(uint32 addr, uint8 data)
{
  ASSERT((addr >= 0x2000) && (addr < 0x2008));

//  LOG("PPU Write " << HEX(addr,4) << " = " << HEX(data,2) << endl);

  LowRegs[addr & 0x0007] = data;

  switch(addr)
  {
    case 0x2000:
      bg_pattern_table_bank_base = (data & 0x10) >> 2;
      spr_pattern_table_bank_base = (data & 0x08) >> 1;
      ppu_addr_inc = (data & 0x04) ? 32 : 1;

      // t:0000110000000000=d:00000011
      loopy_t = (loopy_t & 0xF3FF) | (((uint16)(data & 0x03)) << 10);
      break;

    case 0x2001:
      break;

    case 0x2003:
      spr_ram_rw_ptr = data;
      break;

    case 0x2004:
      ASSERT (current_frame_line == 240);
      spr_ram[spr_ram_rw_ptr++] = data;
      break;

    case 0x2005:
      toggle_2005_2006 = !toggle_2005_2006;

      if(toggle_2005_2006)
      {
        // first write
        
        // t:0000000000011111=d:11111000
        loopy_t = (loopy_t & 0xFFE0) | (((uint16)(data & 0xF8)) >> 3);

        // x=d:00000111
        loopy_x = data & 0x07;
      }
      else
      {
        // second write

        // t:0000001111100000=d:11111000
        loopy_t = (loopy_t & 0xFC1F) | (((uint16)(data & 0xF8)) << 2);
	      
        // t:0111000000000000=d:00000111
        loopy_t = (loopy_t & 0x8FFF) | (((uint16)(data & 0x07)) << 12);
      }
      break;

    case 0x2006:
      toggle_2005_2006 = !toggle_2005_2006;

      if(toggle_2005_2006)
      {
        // first write

	      // t:0011111100000000=d:00111111
	      // t:1100000000000000=0
        loopy_t = (loopy_t & 0x00FF) | (((uint16)(data & 0x3F)) << 8);
      }
      else
      {
        // second write

	      // t:0000000011111111=d:11111111
        loopy_t = (loopy_t & 0xFF00) | ((uint16)data);

	      // v=t
        loopy_v = loopy_t;
      }
      break;

    case 0x2007:
      write_2007(data);
      break;
  }
}

uint8 NES_PPU::Read0x4014()
{
  return HighReg0x4014;
}

void NES_PPU::Write0x4014(uint8 data)
{
  uint32 addr;

//  LOG("PPU Write 0x4014 = " << HEX(data,2) << endl);

  HighReg0x4014 = data;

  addr = ((uint32)data) << 8;

  ASSERT (current_frame_line == 240);
  // do SPR-RAM DMA
  for(uint32 i = 0; i < 256; i++)
  {
    spr_ram[i] = parent_NES->cpu->GetByte(addr++);
  }
}


/* ---------------------------------------- */
#define RENDER_BG_MMC5
#define RENDER_BG_SKIP
#include "render_bg.cpp"
#undef RENDER_BG_SKIP
#include "render_bg.cpp"
#undef RENDER_BG_MMC5

#define RENDER_BG_MMC2
#define RENDER_BG_SKIP
#include "render_bg.cpp"
#undef RENDER_BG_SKIP
#include "render_bg.cpp"
#undef RENDER_BG_MMC2

#define RENDER_BG_BANKSWITCH_PER_TILE
#include "render_bg.cpp"
#undef RENDER_BG_BANKSWITCH_PER_TILE

#include "render_bg.cpp"

/* ---------------------------------------- */
#define RENDER_SPR_MMC5
#define RENDER_SPR_SKIP
#include "render_spr.cpp"
#undef RENDER_SPR_SKIP
#include "render_spr.cpp"
#undef RENDER_SPR_MMC5

#define RENDER_SPR_MMC2
#define RENDER_SPR_SKIP
#include "render_spr.cpp"
#undef RENDER_SPR_SKIP
#include "render_spr.cpp"
#undef RENDER_SPR_MMC2

#include "render_spr.cpp"
