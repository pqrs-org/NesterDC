/* ---------------------------------------- */
void
#if defined (DO_SCANLINE_BANKSWITCH_PER_TILE)
NES_PPU::do_scanline_and_draw_bankswitch_per_tile (uint16 *buf, float CYCLES_PER_DRAW)
#elif defined (DO_SCANLINE_MMC5)
NES_PPU::do_scanline_and_draw_mmc5 (uint16 *buf) 
#elif defined (DO_SCANLINE_MMC2)
NES_PPU::do_scanline_and_draw_mmc2 (uint16 *buf) 
#else
NES_PPU::do_scanline_and_draw (uint16 *buf) 
#endif
{
  if (!bg_enabled()) 
  {
    int i;
    uint32 *p = (uint32 *)(buf + SIDE_MARGIN);
    uint32 bg = (bg_pal_color[0] << 16) | bg_pal_color[0];
    
    for (i = 0; i < (NES_SCREEN_WIDTH / 16 / 2); ++i)
    {
      *p++ = bg; *p++ = bg; *p++ = bg; *p++ = bg; 
      *p++ = bg; *p++ = bg; *p++ = bg; *p++ = bg; 
      *p++ = bg; *p++ = bg; *p++ = bg; *p++ = bg; 
      *p++ = bg; *p++ = bg; *p++ = bg; *p++ = bg; 
    }
  }
  
  if (bg_or_spr_enabled ())
  {
    LOOPY_SCANLINE_START(loopy_v, loopy_t);
    
    if (bg_enabled())
    {
#ifdef DO_SCANLINE_BANKSWITCH_PER_TILE
      render_bg_bankswitch_per_tile (buf, CYCLES_PER_DRAW);
#elif defined (DO_SCANLINE_MMC5)
      render_bg_mmc5 (buf);
#elif defined (DO_SCANLINE_MMC2)
      render_bg_mmc2 (buf);
#else
      render_bg_regular (buf);
#endif
    }
    else
    {
      int i;
      uint32 *s = (uint32 *)(solid_buf);
      
      *s++ = 0; *s++ = 0;
      for (i = 0; i < NES_SCREEN_WIDTH / 8 / 4; ++i)
      {
	*s++ = 0; *s++ = 0; 
	*s++ = 0; *s++ = 0; 
	*s++ = 0; *s++ = 0; 
	*s++ = 0; *s++ = 0; 
      }
      *s++ = 0; *s++ = 0;
      
#ifdef DO_SCANLINE_BANKSWITCH_PER_TILE
      parent_NES->emulate_CPU_cycles(CYCLES_PER_DRAW);
#endif
    }
    
    if (spr_enabled ())
    {
#if defined (DO_SCANLINE_MMC5)
      render_spr_mmc5 (buf);
#elif defined (DO_SCANLINE_MMC2)
      render_spr_mmc2 (buf);
#else
      render_spr_regular (buf);
#endif
    }
    
    LOOPY_NEXT_LINE (loopy_v);
  }
  
  current_frame_line++;
}


void
#if defined (DO_SCANLINE_BANKSWITCH_PER_TILE)
NES_PPU::do_scanline_and_dont_draw_bankswitch_per_tile(float CYCLES_PER_DRAW)
#elif defined (DO_SCANLINE_MMC5)
NES_PPU::do_scanline_and_dont_draw_mmc5()
#elif defined (DO_SCANLINE_MMC2)
NES_PPU::do_scanline_and_dont_draw_mmc2()
#else
NES_PPU::do_scanline_and_dont_draw()
#endif
{
  // if sprite 0 flag not set and sprite 0 on current line
  if ((!sprite0_hit()) &
      (!is_sprite_out_of_line (spr_ram, current_frame_line, sprite_height())))
  {
    // render line to dummy buffer
#if defined (DO_SCANLINE_BANKSWITCH_PER_TILE)
    do_scanline_and_draw_bankswitch_per_tile (dummy_buffer, CYCLES_PER_DRAW);
#elif defined (DO_SCANLINE_MMC5)
    do_scanline_and_draw_mmc5 (dummy_buffer);
#elif defined (DO_SCANLINE_MMC2)
    do_scanline_and_draw_mmc2 (dummy_buffer);
#else
    do_scanline_and_draw (dummy_buffer);
#endif
  }
  else
  {
    if (bg_or_spr_enabled())
    {
      LOOPY_SCANLINE_START(loopy_v, loopy_t);
      
#if defined (DO_SCANLINE_BANKSWITCH_PER_TILE)
      if (bg_enabled ())
	parent_NES->emulate_CPU_cycles (CYCLES_PER_DRAW);
#elif defined (DO_SCANLINE_MMC5)
      if (bg_enabled ())
	render_bg_mmc5_skip ();
      if (spr_enabled ())
	render_spr_mmc5_skip ();
#elif defined (DO_SCANLINE_MMC2)
      if (bg_enabled ())
	render_bg_mmc2_skip ();
      if (spr_enabled ())
	render_spr_mmc2_skip ();
#endif
      
      LOOPY_NEXT_LINE(loopy_v);
    }
    current_frame_line++;
  }
}


