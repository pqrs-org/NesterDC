#ifdef RENDER_BG_BANKSWITCH_PER_TILE
    parent_NES->emulate_CPU_cycles(CYCLES_PER_DRAW / 33);
#endif

#ifndef RENDER_BG_SKIP
    /* set palette index (see above) */
    {
      uint8 shift = ((tile_y & 0x02) << 1) | (tile_x & 0x02);
      pattern_palette = bg_pal_color +
        (((PPU_VRAM_banks[attr_bank][attr_addr] >> shift) & 0x03) << 2);
    }
#endif

    uint8 p_addr = PPU_VRAM_banks[name_bank][name_addr];
    pattern_addr = ((p_addr & 0x3f) << 4) + tile_y_bit;
    pattern_bank = bg_pattern_table_bank_base + (p_addr >> 6);

#ifndef RENDER_BG_SKIP
    {
      uint8 t1 = PPU_VRAM_banks[pattern_bank][pattern_addr + 8];
      uint8 t2 = PPU_VRAM_banks[pattern_bank][pattern_addr];
      uint8 p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
      uint8 p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);

      p += 16;
      s += 16;
      
      *(--p) = pattern_palette[*(--s) = (p2 & 0x03)]; /* 7 */
      *(--p) = pattern_palette[*(--s) = (p1 & 0x03)]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      *(--p) = pattern_palette[*(--s) = (p2 & 0x03)]; /* 5 */
      *(--p) = pattern_palette[*(--s) = (p1 & 0x03)]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      *(--p) = pattern_palette[*(--s) = (p2 & 0x03)]; /* 3 */
      *(--p) = pattern_palette[*(--s) = (p1 & 0x03)]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      *(--p) = pattern_palette[*(--s) = (p2 & 0x03)]; /* 1 */
      *(--p) = pattern_palette[*(--s) = (p1 & 0x03)]; /* 0 */
    }
#endif
    
#ifdef RENDER_BG_MMC2
    {
      /* if ((addr & 0x0FF0) == 0x0FD0 || (addr & 0x0FF0) == 0x0FE0) */
      uint32 latch_bank = (pattern_addr >> 4) - 0x3d;
      
      if (((pattern_bank & 0x3) == 0x3) & 
          ((latch_bank & ~1) == 0))
        parent_NES->mapper->PPU_Latch_FDFE(pattern_bank & 0x4, latch_bank);
    }
#endif

