    /* make tile_pattern */ 
    uint32 tile_addr;
    uint32 tile_bank;
    uint32 y;

    y = current_frame_line - (*spr + 1);

    /* vertically flip */
    if (spr[2] & 0x80)
      y = (spr_height - 1) - y;
    
    if (spr_height == 8)
    {
      tile_addr = ((spr[1] & 0x3f) << 4) + (y & 0x07);
      tile_bank = spr_pattern_table_bank_base + (spr[1] >> 6);
    }
    else
    {
      tile_bank = ((spr[1] & 0x1) << 2) + (spr[1] >> 6);
      tile_addr = ((spr[1] & 0x3e) << 4) + (y & 0x7) + ((y & 0x8) << 1);
    }

    uint8 t1, t2;
    t1 = PPU_VRAM_banks[tile_bank][tile_addr + 8];
    t2 = PPU_VRAM_banks[tile_bank][tile_addr];
    
    uint8 rendered_tile[8];
    uint8 *t;
    uint8 p1, p2;

    /* horizontally filp */
    if (spr[2] & 0x40)
    {
      t = rendered_tile;
      
      if ((spr[3] < 8) & spr_clip_left8 ())
      {
        t1 &= (0xff << (8 - spr[3]));
        t2 &= (0xff << (8 - spr[3]));
      }
      
      p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
      p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
      
      *t++ = p2 & 0x03; /* 0 */
      *t++ = p1 & 0x03; /* 1 */
      p2 >>= 2; p1 >>= 2;
      
      *t++ = p2 & 0x03; /* 2 */
      *t++ = p1 & 0x03; /* 3 */
      p2 >>= 2; p1 >>= 2;
      
      *t++ = p2 & 0x03; /* 4 */
      *t++ = p1 & 0x03; /* 5 */
      p2 >>= 2; p1 >>= 2;
      
      *t++ = p2 & 0x03; /* 6 */
      *t++ = p1 & 0x03; /* 7 */
      p2 >>= 2; p1 >>= 2;
    }
    else
    {
      t = rendered_tile + 8;
      
      if ((spr[3] < 8) & spr_clip_left8 ())
      {
        t1 &= (0xff >> (8 - spr[3]));
        t2 &= (0xff >> (8 - spr[3]));
      }
      
      p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
      p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
      
      *(--t) = p2 & 0x03; /* 7 */
      *(--t) = p1 & 0x03; /* 6 */
      p2 >>= 2; p1 >>= 2;
      
      *(--t) = p2 & 0x03; /* 5 */
      *(--t) = p1 & 0x03; /* 4 */
      p2 >>= 2; p1 >>= 2;
      
      *(--t) = p2 & 0x03; /* 3 */
      *(--t) = p1 & 0x03; /* 2 */
      p2 >>= 2; p1 >>= 2;
      
      *(--t) = p2 & 0x03; /* 1 */
      *(--t) = p1 & 0x03; /* 0 */
      p2 >>= 2; p1 >>= 2;
    }
    
    uint16 *p = buf + SIDE_MARGIN + spr[3];
    uint8 *s = solid_buf + SIDE_MARGIN + spr[3];
    uint16 *palette = spr_pal_color + ((spr[2] & 0x03) << 2);
    t = rendered_tile;
    uint32 solid_mask;
    
    if (spr[2] & 0x20)
      solid_mask = SPR_WRITTEN_FLAG | BG_WRITTEN_FLAG; /* lower priority */
    else
      solid_mask = SPR_WRITTEN_FLAG;

    uint8 written; /* 0, 2..30 */
    uint8 is_spr;  /* 0 or 16. ("16" if sprite draw) */ 

    /* 0 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 1 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 2 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 3 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 4 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 5 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 6 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
    /* 7 */
    written = (*s & solid_mask) << 1;
    is_spr = ((*t != 0) & !written) << 4;
    *s++ |= *t << 2;
    *p++ = (*p << is_spr) | palette[*t++ >> written];
