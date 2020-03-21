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

#ifndef NESPPU_H_
#define NESPPU_H_

#include <stdlib.h>
#include <stdio.h>
#include "types.h"

class NES;  // class prototype
class NES_SNSS;

class NES_PPU
{
  // SNSS functions
  friend class NES_SNSS;
  
public:
  // SIDE_MARGIN allocates 2 8 pixel columns on the left and the right
  // for optimized NES background drawing
  enum { 
    NES_SCREEN_WIDTH  = 256,
    NES_SCREEN_HEIGHT = 240,

    SIDE_MARGIN = 8,

    NES_SCREEN_WIDTH_VIEWABLE  = NES_SCREEN_WIDTH,

    NES_BACKBUF_WIDTH = NES_SCREEN_WIDTH + (2*SIDE_MARGIN)
  };
  
  enum mirroring_type
  {
    MIRROR_HORIZ,
    MIRROR_VERT,
    MIRROR_FOUR_SCREEN
  };

  ~NES_PPU() {}
  bool initialize(NES* parent);

  void reset();
  void set_palette (); /* need for SNSS */
  
  void set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3);
  void set_mirroring(mirroring_type m);
  
  uint8 ReadLowRegs(uint32 addr);
  void  WriteLowRegs(uint32 addr, uint8 data);
  
  uint8 Read0x4014();
  void  Write0x4014(uint8 data);
  
  // these are the rendering functions
  // screen is drawn a line at a time
  void start_frame();
  
  void do_scanline_and_draw_mmc5 (uint16 *buf);
  void do_scanline_and_draw_mmc2 (uint16 *buf);
  void do_scanline_and_draw_bankswitch_per_tile (uint16 *buf, float CYCLES_PER_DRAW);
  void do_scanline_and_draw(uint16* buf);
  
  void do_scanline_and_dont_draw_mmc5();
  void do_scanline_and_dont_draw_mmc2();
  void do_scanline_and_dont_draw_bankswitch_per_tile(float CYCLES_PER_DRAW);
  void do_scanline_and_dont_draw();
  
  void end_frame();
  
  void start_vblank();
  void end_vblank();
  
  // 0x2000
  uint32 NMI_enabled()  { return LowRegs[0] & 0x80; }
  uint32 sprite_height() { return (LowRegs[0] & 0x20) ? 16 : 8; }
  
  // 0x2001
  uint32 spr_enabled()    { return LowRegs[1] & 0x10; }
  uint32 bg_enabled()     { return LowRegs[1] & 0x08; }
  uint32 bg_or_spr_enabled() { return LowRegs[1] & 0x18; }
  uint32 spr_clip_left8() { return !(LowRegs[1] & 0x04); }
  uint32 bg_clip_left8()  { return !(LowRegs[1] & 0x02); }
  
  // 0x2002
  uint32 sprite0_hit()                     { return LowRegs[2] & 0x40; }
  uint32 is_sprite_out_of_line (uint8 *spr, uint32 line, uint32 height) {
    /* !(0 <= line - *spr - 1 < 8)    (height = 8 : 16) */
    return (line - *spr - 1) & ~(height - 1);
  }
  uint32 VRAM_accessible()                 { return LowRegs[2] & 0x10; }
  
  // by rinao
  uint8* get_patt() { return PPU_patterntables; }
  uint8* get_namt() { return PPU_nametables; }
  
  /* return 1 if VRAM, 0 if VROM */
  uint32 get_VRAM_bank_type(int i)
  {
    uint8 *addr = PPU_VRAM_banks[i];
    
    bool in_patterntable = (PPU_patterntables <= addr) & (addr < PPU_patterntables + 0x8000);
    bool in_nametable = (PPU_nametables <= addr) & (addr < PPU_nametables + 4 * 0x400);
    return in_patterntable | in_nametable;
  }
  
  // vram / PPU ram

  // bank ptr table
  // 0-7     = pattern table
  // 8       = name table 0
  // 9       = name table 1
  // A       = name table 2
  // B       = name table 3
  // THE FOLLOWING IS SPECIAL-CASED AND NOT PHYSICALLY IN THE BANK TABLE
  // C       = mirror of name table 0
  // D       = mirror of name table 1
  // E       = mirror of name table 2
  // F       = mirror of name table 3 (0x3F00-0x3FFF are palette info)
  uint8* PPU_VRAM_banks[12];
  
  uint8 bg_pal[0x10];
  uint8 spr_pal[0x10];
  uint16 bg_pal_color[0x10];
  uint16 spr_pal_color[0x10];
  
  uint8 vram_write_protect;
protected:
  NES* parent_NES;
  
  // internal registers
  uint8 LowRegs[0x08];
  uint8 HighReg0x4014;
  
  // 2 VRAM pattern tables
  uint8 PPU_patterntables[0x8000]; /* expand for NES_mapper::set_VRAM_bank */
  // 4 internal name tables (2 of these really are in the NES)
  uint8 PPU_nametables[4*0x400];
  
  // these functions read from/write to VRAM using loopy_v
  uint8 read_2007();
  void write_2007(uint8 data);
  
  uint32  in_vblank;
  
  uint16  bg_pattern_table_bank_base;
  uint16  spr_pattern_table_bank_base;
  
  uint16  ppu_addr_inc;
  
  // loopy's internal PPU variables
  uint16  loopy_v;  // vram address -- used for reading/writing through $2007
                    // see loopy-2005.txt
  uint16  loopy_t;  // temp vram address
  uint8   loopy_x;  // 3-bit subtile x-offset

  uint8   toggle_2005_2006;

  uint8 spr_ram_rw_ptr;  // sprite ram read/write pointer
  uint8 spr_ram[0x100];

  uint8 read_2007_buffer;
  
  // rendering stuff
  uint32 current_frame_line;
  
  /* BG_WRITTEN_FLAG == pattern_color. */
  enum { 
    BG_WRITTEN_FLAG = 0x03, 
    SPR_WRITTEN_FLAG = 0x0c,
  };
  /* bit flags for pixels of current line */
  uint8 solid_buf[NES_BACKBUF_WIDTH];
  /* used to do sprite 0 hit detection when we aren't supposed to draw */
  uint16 dummy_buffer[NES_BACKBUF_WIDTH];
  
  struct sprites_per_line
  {
    uint8 num;
    uint8 sprite_index[63]; /* exclude #0 */
  } spr_per_line[256 + 32]; /* last 32 is guard */
  
  void render_bg_regular (uint16 *buf);
  void render_bg_mmc5 (uint16 *buf);
  void render_bg_mmc5_skip ();
  void render_bg_mmc2 (uint16 *buf);
  void render_bg_mmc2_skip ();
  void render_bg_bankswitch_per_tile (uint16 *buf, float CYCLES_PER_DRAW);
  
  void render_spr_regular (uint16 *buf);
  void render_spr_mmc5_skip ();
  void render_spr_mmc5 (uint16 *buf);
  void render_spr_mmc2_skip ();
  void render_spr_mmc2 (uint16 *buf);
};

#endif


