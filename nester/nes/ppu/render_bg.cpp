void
#if defined (RENDER_BG_MMC5)
#  ifdef RENDER_BG_SKIP
NES_PPU::render_bg_mmc5_skip ()
#  else
NES_PPU::render_bg_mmc5 (uint16 *buf)
#  endif
#elif defined (RENDER_BG_MMC2)
#  ifdef RENDER_BG_SKIP
NES_PPU::render_bg_mmc2_skip ()
#  else
NES_PPU::render_bg_mmc2 (uint16 *buf)
#  endif
#elif defined (RENDER_BG_BANKSWITCH_PER_TILE)
NES_PPU::render_bg_bankswitch_per_tile (uint16 *buf, float CYCLES_PER_DRAW)
#else
NES_PPU::render_bg_regular (uint16 *buf)
#endif
{
  int loop_before_switch;
  int loop_after_switch;
  
  uint32 tile_x;
  uint32 tile_y;
  uint32 tile_y_bit;
  
  uint32 name_addr;
  uint32 name_bank;
  uint32 attr_addr;
  uint32 attr_bank;
  
  uint32 pattern_addr;
  uint32 pattern_bank;
  
#ifndef RENDER_BG_SKIP
  uint16 *p;
  uint8 *s;
  uint16 *pattern_palette;
#endif
  
  tile_x = (loopy_v & 0x1f);
  tile_y = (loopy_v >> 5) & 0x1f;
  tile_y_bit = (loopy_v >> 12) & 0x7;
  
  name_addr = (loopy_v & 0x3ff);
  attr_addr = 0x3c0 + ((tile_y & 0x1c) << 1) + (tile_x >> 2);
  name_bank = attr_bank = 8 + ((loopy_v >> 10) & 0x3);
  
#ifndef RENDER_BG_SKIP  
  p = buf       + (SIDE_MARGIN - loopy_x);
  s = solid_buf + (SIDE_MARGIN - loopy_x); 
  
  /* adjust before loop */
  p -= 8;
  s -= 8;
#endif
  
#ifdef RENDER_BG_MMC5
  parent_NES->mapper->PPU_Latch_RenderScreen(1, 0);
#endif
  
  loop_before_switch = 32 - tile_x;
  loop_after_switch = 33 - loop_before_switch;
  
  while (loop_before_switch--)
  {
#include "render_tile.cpp"
    ++tile_x;
    ++name_addr;
    attr_addr += ((tile_x & 0x3) == 0);
  }
  
  tile_x = 0;
  name_addr -= 32;
  name_bank ^= 1;
  attr_addr -= 8;
  attr_bank ^= 1;
  
  while (loop_after_switch--)
  {
#include "render_tile.cpp"
    ++tile_x;
    ++name_addr;
    attr_addr += ((tile_x & 0x3) == 0);
  }
  
#ifndef RENDER_BG_SKIP  
  {
    uint32 *p32, *s32;
    
    /* clear SIDE_MARGIN */
    s32 = (uint32 *)(solid_buf);
    *s32++ = 0; *s32++ = 0;
    
    if(bg_clip_left8())
    {
      /* clip left 8 pixels */
      p32 = (uint32 *)(buf + SIDE_MARGIN);
      
      *p32++ = 0; *p32++ = 0; *p32++ = 0; *p32++ = 0;
      *s32++ = 0; *s32++ = 0;
    }
    
    /* clear SIDE_MARGIN */
    s32 = (uint32 *)(solid_buf + SIDE_MARGIN + NES_SCREEN_WIDTH);
    *s32++ = 0; *s32++ = 0;
  }
#endif
}


