#ifndef __NES_EMULATE_FRAME_CPP__
#define __NES_EMULATE_FRAME_CPP__

/*
 *NOTE* 
 
 rendered resolution = 272*240 
 NES resolution = 256*240, so there are 8 pixels of 
 garbage on each side of the screen 
*/


void
NES::emulate_frame_skip_line_generic (int i)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i); 
  ppu->do_scanline_and_dont_draw(); 
}


void
NES::emulate_frame_skip_line_mmc5 (int i)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i); 
  ppu->do_scanline_and_dont_draw_mmc5(); 
}


void
NES::emulate_frame_skip_line_mmc2 (int i)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i); 
  ppu->do_scanline_and_dont_draw_mmc2(); 
}


void
NES::emulate_frame_skip_line_bankswitch_per_tile (int i)
{
  ppu->do_scanline_and_dont_draw_bankswitch_per_tile (CYCLES_PER_LINE * 32 / 42); 
  
  /* do half line's worth of CPU cycles (hblank) */ 
  emulate_CPU_cycles(13); 
  mapper->HSync(i); 
  emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13); 
}


void
NES::emulate_frame_line_generic (int i, uint16 *cur_line)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i);
  ppu->do_scanline_and_draw(cur_line);
}


void
NES::emulate_frame_line_mmc5 (int i, uint16 *cur_line)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i);
  ppu->do_scanline_and_draw_mmc5(cur_line);
}


void
NES::emulate_frame_line_mmc2 (int i, uint16 *cur_line)
{
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(i);
  ppu->do_scanline_and_draw_mmc2(cur_line);
}


void
NES::emulate_frame_line_bankswitch_per_tile (int i, uint16 *cur_line)
{
  /* render line */
  ppu->do_scanline_and_draw_bankswitch_per_tile (cur_line, 
						 CYCLES_PER_LINE * 32 / 42); 
  /* do half line's worth of CPU cycles (hblank) */ 
  emulate_CPU_cycles(13); 
  mapper->HSync(i); 
  emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13); 
}


#endif /* __NES_EMULATE_FRAME_CPP__ */

void 
#ifdef EMULATE_FRAME
NES::emulate_frame (uint16 *screen)
#else
NES::emulate_frame_skip ()
#endif
{
  int i;
  
  if (nes_nnnesterj_cheatinfo_alive_num > 0)
    apply_nnnesterj_cheat ();
  
  trim_cycle_counts();
  
  ppu->start_frame();
  
#ifdef EMULATE_FRAME

#define EMULATE_FRAME_BODY(func_type) \
  { \
 \
    for (i = 0; i < 8; ++i) \
      emulate_frame_skip_line_##func_type (i); \
 \
    for (i = 8; i < 8 + 224; ++i) \
    { \
      emulate_frame_line_##func_type (i, screen); \
      screen += 320; \
    } \
 \
    for (i = 8 + 224; i < 8 + 224 + 8; ++i) \
      emulate_frame_skip_line_##func_type (i); \
  }
  
  if (BANKSWITCH_PER_TILE)
  {
    mapper->HSync (0);
    
    EMULATE_FRAME_BODY(bankswitch_per_tile);
  }
  else if (ROM->get_mapper_num () == 5)
  {
    EMULATE_FRAME_BODY(mmc5);
  }
  else if (ROM->get_mapper_num () == 9 ||
	   ROM->get_mapper_num () == 10)
  {
    EMULATE_FRAME_BODY(mmc2);
  }  
  else
  {
    EMULATE_FRAME_BODY(generic);
  }
#else

#define EMULATE_FRAME_SKIP(func_type) \
  { \
  for (i = 0; i < 240; ++i) \
    emulate_frame_skip_line_##func_type (i); \
  }
  
  if (BANKSWITCH_PER_TILE)
  {
    mapper->HSync (0);
    EMULATE_FRAME_SKIP(bankswitch_per_tile);
  }
  else if (ROM->get_mapper_num () == 5)
  {
    EMULATE_FRAME_SKIP(mmc5);
  }
  else if (ROM->get_mapper_num () == 9 ||
	   ROM->get_mapper_num () == 10)
  {
    EMULATE_FRAME_SKIP(mmc2);
  }
  else
  {
    EMULATE_FRAME_SKIP(generic);
  }
#endif
  
  ppu->end_frame();
  
  if (!(frame_irq_enabled & 0xC0))
    cpu->DoPendingIRQ();
  
  /* 240 */
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(240);
  
  /* 241 */
  ppu->start_vblank();
  mapper->VSync();
  emulate_CPU_cycles(CYCLES_BEFORE_NMI);
  if (ppu->NMI_enabled()) cpu->DoNMI();
  emulate_CPU_cycles(CYCLES_PER_LINE - CYCLES_BEFORE_NMI);
  mapper->HSync(241);
  
  for (i = 242; i < 261; i++)
  {
    emulate_CPU_cycles(CYCLES_PER_LINE);
    mapper->HSync(i);
  }
  
  /* 261 */
  ppu->end_vblank ();
  emulate_CPU_cycles(CYCLES_PER_LINE);
  mapper->HSync(261);
  
  apu->DoFrame();
  apu->SyncAPURegister();
}

