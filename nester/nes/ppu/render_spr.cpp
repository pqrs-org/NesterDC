void
#if defined (RENDER_SPR_MMC5)
#  ifdef RENDER_SPR_SKIP
NES_PPU::render_spr_mmc5_skip()
#  else
NES_PPU::render_spr_mmc5 (uint16 *buf)
#  endif
#elif defined (RENDER_SPR_MMC2)
#  ifdef RENDER_SPR_SKIP
NES_PPU::render_spr_mmc2_skip()
#  else
NES_PPU::render_spr_mmc2 (uint16 *buf)
#  endif
#else
NES_PPU::render_spr_regular (uint16 *buf)
#endif
{
  uint8* spr;
  int num_sprites;
  uint32 spr_height = sprite_height ();
  
  /* pre-calced sprite data */
  sprites_per_line *spl = spr_per_line + current_frame_line;
  uint8 *spr_index = spl->sprite_index;
  
#ifdef RENDER_SPR_MMC5
  parent_NES->mapper->PPU_Latch_RenderScreen(0,0);
#endif
  
  /* draw #0 sprites */
  if (!is_sprite_out_of_line (spr_ram, current_frame_line, spr_height))
  {
    spr = spr_ram;
    
#ifdef RENDER_SPR_MMC2
    {
      /* if ((addr & 0x0FF0) == 0x0FD0 || (addr & 0x0FF0) == 0x0FE0) */
      uint32 latch_bank = spr[1] - 0xfd;
      
      if ((latch_bank & 0xfe) == 0)
	parent_NES->mapper->PPU_Latch_FDFE(0, latch_bank);
    }
#endif
    
#ifndef RENDER_SPR_SKIP
#include "render_spr_tile.cpp"

    /* set sprite #0 hit flag */
    if (!(LowRegs[2] & 0x40))
    {
      s = solid_buf + SIDE_MARGIN + spr[3];
      int flag = 0;
      
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      flag |= ((*s & SPR_WRITTEN_FLAG) != 0) & ((*s & BG_WRITTEN_FLAG) != 0); ++s;
      
      LowRegs[2] |= flag << 6;
    }

#endif
  }
  
  num_sprites = spl->num;
  while (num_sprites--)
  {
    spr = spr_ram + (*spr_index << 2);
    
#ifdef RENDER_SPR_MMC2
    {
      /* if ((addr & 0x0FF0) == 0x0FD0 || (addr & 0x0FF0) == 0x0FE0) */
      int latch_bank = spr[1] - 0xfd;
      if ((latch_bank & 0xfe) == 0)
	parent_NES->mapper->PPU_Latch_FDFE(0, latch_bank);
    }
#endif
    
#ifndef RENDER_SPR_SKIP
#include "render_spr_tile.cpp"
#endif
    ++spr_index;
  }

  if (spl->num > 8)
    LowRegs[2] |= 0x20;
  else
    LowRegs[2] &= 0xDF;
}


