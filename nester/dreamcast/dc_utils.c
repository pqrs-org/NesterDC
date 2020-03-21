#include <kos.h>
#include <stdio.h>
#include "dc_utils.h"
#include "dc_vmu.h"


/* ============================================================ */
/* sq */

#include <dc/sq.h>


/* n must be multiple of 64 */
void
dc_sq_cpy(void *dest, void *src, int n)
{
  uint32 *sq;
  uint32 *d, *s;
  
  d = (uint32 *)(0xe0000000 | (((uint32)dest) & 0x03ffffe0));
  s = (uint32 *)(src);
  
  /* Set store queue memory area as desired */
  QACR0 = ((((uint32)dest)>>26)<<2)&0x1c;
  QACR1 = ((((uint32)dest)>>26)<<2)&0x1c;
  
  n >>= 6;
  while (n--) 
  {
    /* sq0 */ 
    sq = d;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    asm("pref @%0" : : "r" (d));
    d += 8;
    
    /* sq1 */
    sq = d;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    asm("pref @%0" : : "r" (d));
    d += 8;
  }
}


void
dc_wait_sq_cpy_done ()
{
  /* wait for both store queues to complete */
  *((uint32 *)(0xe0000000)) = 0;
  *((uint32 *)(0xe0000020)) = 0;
}


/* ============================================================ */
/* controller handling */
dc_maple_info_t dc_maple_controller_info[DC_MAPLE_INFO_SIZE];
dc_maple_info_t dc_maple_mouse_info[DC_MAPLE_INFO_SIZE];
dc_maple_info_t dc_maple_keyboard_info[DC_MAPLE_INFO_SIZE];


void
dc_maple_init ()
{
  int n;
  maple_device_t *dev;
  dc_maple_info_t *p;
  
  struct _typemap {
    char *name;
    dc_maple_controller_type_t type;
  } typemap[] = {
    {"Arcade Stick", DC_MAPLE_CONTROLLER_ARCADESTICK},
    {NULL, DC_MAPLE_CONTROLLER_NORMAL},
  };
  struct _typemap *tp;
  
  /* controller */
  n = 0;
  p = dc_maple_controller_info;
  for (;;)
  {
    dev = maple_enum_type (n, MAPLE_FUNC_CONTROLLER);
    p->dev = dev;
    p->type.c_type = DC_MAPLE_CONTROLLER_NORMAL;
    
    tp = typemap;
    while (tp->name)
    {
      if (!strncmp(tp->name, dev->info.product_name, strlen(tp->name)))
      {
        p->type.c_type = tp->type;
        break;
      }
      
      ++tp;
    }
    
    ++n;
    ++p;
    
    if (!dev) break;
  }
  
  /* mouse */
  n = 0;
  p = dc_maple_mouse_info;
  for (;;)
  {
    dev = maple_enum_type (n, MAPLE_FUNC_MOUSE);
    p->dev = dev;
    p->type.m_type = DC_MAPLE_MOUSE_3BUTTON;
    
    ++n;
    ++p;
    
    if (!dev) break;
  }
  
  /* keyboard */
  n = 0;
  p = dc_maple_keyboard_info;
  for (;;)
  {
    dev = maple_enum_type (n, MAPLE_FUNC_KEYBOARD);
    p->dev = dev;
    p->type.k_type = DC_MAPLE_KEYBOARD_JP106;
    
    ++n;
    ++p;
    
    if (!dev) break;
  }
}


/* ------------------------------------------------------------ */
/* screen rect */

dc_screen_offset_t dc_screen_offset = {0, 16};


void
dc_screen_offset_init ()
{
  dc_screen_offset.x = 0;
  dc_screen_offset.y = 16;
}


/* ============================================================ */
#include "lcd_icons/3x5fonts.h"

void
dc_lcd_icon_clear (uint8 *screen, int is_black)
{
  if (is_black)
    memset(screen, '+', 48 * 32);
  else
    memset(screen, '.', 48 * 32);
}


void
dc_lcd_icon_draw_char (uint8 *screen, int x, int y, int ch)
{
  const char *font;
  int x1, y1;
  
  if (ch <= 31 || ch >= 127) return;
  if (x < 0 || x >= 48 - 3) return;
  if (y < 0 || y >= 32 - 5) return;
  
  font = vmu_3x5fonts + 3 * 5 * (ch - 32);
  screen += y * 48 + x;
  
  for (y1 = 0; y1 < 5; ++y1)
  {
    for (x1 = 0; x1 < 3; ++x1)
    {
      if (*font == '#')
        *screen = '.';
      ++font;
      ++screen;
    }
    screen += 48 - 3;
  }
}


void
dc_lcd_icon_draw_string (uint8 *screen, int x, int y, const char *str)
{
  while (*str) 
  {
    dc_lcd_icon_draw_char(screen, x, y, *str);
    x += 4;
    str++;
  }
}


void
dc_lcd_icon_flip(const uint8 *screen) 
{
  uint8 bitmap[(48 / 8) * 32];
  uint8 *b;
  const char *v;
  int tile_x, y, i;
  
  memset (bitmap, 0, sizeof(bitmap));
  
  b = bitmap;
  v = screen + 48 * 32;
  for (y = 0; y < 32; y++)
  {
    /* draw (8x1 tile) x 6 */
    for (tile_x = 0; tile_x < 6; ++tile_x)
    {
      uint8 t = 0;
      
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      t <<= 1; t |= (*(--v) == '+');
      
      *b++ = t;
    }
  }
  
  i = 0;
  for (;;)
  {
    maple_device_t *dev;
    
    dev = maple_enum_type (i++, MAPLE_FUNC_LCD);
    if (!dev) break;
    
    vmu_draw_lcd (dev, bitmap);
  }
}


void
dc_lcd_clear ()
{
  uint8 screen[48 * 32];
  
  dc_lcd_icon_clear (screen, 0);
  dc_lcd_icon_flip (screen);
}



/* ------------------------------------------------------------ */
/* VMU utility */

maple_device_t *
dc_vmu_search_file (const char *filename)
{
  int n;
  maple_device_t *dev;
  
  n = 0;
  for (;;)
  {
    dev = maple_enum_type (n, MAPLE_FUNC_MEMCARD);
    if (!dev) return NULL;
    
    if (ndc_vmu_get_dirent (NULL, dev, filename) >= 0)
      return dev;
    
    ++n;
  }
}



/* ------------------------------------------------------------ */
int
load_bmp (uint16 *raw, const char *filename)
{
  FILE *fp = NULL;
  uint8 bmp[320 * 240 * 3];
  uint8 *p;
  int i;
  
  fp = fopen(filename, "r");
  if (!fp) goto error;
  
  fseek(fp, 54, SEEK_SET);
  if (fread(bmp, sizeof(bmp), 1, fp) != 1) 
    goto error;
  
  fclose(fp);
  
  p = bmp;
  for (i = 0; i < 320 * 240; ++i)
  {
    uint16 r, g, b;
    
    b = *p++ * 32 / 256;
    g = *p++ * 64 / 256;
    r = *p++ * 32 / 256;
    
    *raw++ = (r << 11) | (g << 5) | (b << 0);
  }
  
  return 0;
  
error:
  if (fp) fclose(fp);
  memset (raw, 0, 320 * 240 * 2);
  return -1;
}


void
dc_bmpimage_copy_texture_320x240to512x256 (pvr_ptr_t txr_addr, uint16 *raw)
{
  int h;
  uint16 *d = txr_addr;
  
  for (h = 0; h < 240; ++h)
  {
    dc_sq_cpy (d, raw, 320 * 2);
    d += 512;
    raw += 320;
  }
  dc_wait_sq_cpy_done ();
}


void
dc_bmpimage_commit_texture_320x240on512x256 (pvr_poly_hdr_t *poly)
{
  pvr_vertex_t vert;
  
  pvr_prim (poly, sizeof(pvr_poly_hdr_t));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = dc_screen_offset.x;
  vert.y = dc_screen_offset.y + 480;
  vert.z = 256;
  vert.u = 0;
  vert.v = 0;
  vert.argb = 0xffffffff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x;
  vert.y = dc_screen_offset.y;
  vert.u = 0;
  vert.v = 240.0/256;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + 640;
  vert.y = dc_screen_offset.y + 480;
  vert.u = 320.0/512;
  vert.v = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = dc_screen_offset.x + 640;
  vert.y = dc_screen_offset.y;
  vert.u = 320.0/512;
  vert.v = 240.0/256;
  pvr_prim (&vert, sizeof(vert));
}


/* ============================================================ */
static pvr_poly_hdr_t dc_pvr_font_poly_hdr;

/*
  load 12x24 font. (use as 14x26).
*/

#define DC_PVR_FONT_WIDTH 14
#define DC_PVR_FONT_HEIGHT 26

void
dc_pvr_font_init ()
{
  pvr_ptr_t txr_font;
  pvr_poly_cxt_t poly;
  int x, y;
  int ch;
  uint16 font_image[256 * 256] __attribute__ ((aligned (32)));;
  
  /* setup font texture */
  txr_font = pvr_mem_malloc (256 * 256 * 2);
  pvr_poly_cxt_txr (&poly, PVR_LIST_TR_POLY,
                    PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED,
                    256, 256, txr_font, PVR_FILTER_NONE);
  pvr_poly_compile (&dc_pvr_font_poly_hdr, &poly);
  
  /* draw font to texture */
  memset(font_image, 0, sizeof(font_image));
  ch = 0;
  for (y = 0; y < 6; ++y)
  {
    for (x = 0; x < 16; ++x)
    {
      uint16 *p;
      
      p = font_image + 256 * y * DC_PVR_FONT_HEIGHT + x * DC_PVR_FONT_WIDTH;
      bfont_draw_thin (p, 256, 0, 32 + ch, 0);
      bfont_draw_thin (p + 1, 256, 0, 32 + ch, 0);
      
      ++ch;
    }
  }
  
  dc_sq_cpy (txr_font, font_image, sizeof(font_image));
  dc_wait_sq_cpy_done();
}


void
dc_pvr_font_commit_start ()
{
  pvr_prim (&dc_pvr_font_poly_hdr, sizeof(pvr_poly_hdr_t));
}


static void
dc_pvr_font_commit_char (float x, float y, float z, uint32 argb, char ch)
{
  pvr_vertex_t vert;
  int cx, cy;
  float u1, v1, u2, v2;
  
  cx = ((ch - 32) & 15) * DC_PVR_FONT_WIDTH;
  cy = ((ch - 32) >> 4) * DC_PVR_FONT_HEIGHT;
  u1 = cx * 1.0 / 256;
  v1 = cy * 1.0 / 256;
  u2 = (cx + DC_PVR_FONT_WIDTH) * 1.0 / 256;
  v2 = (cy + DC_PVR_FONT_HEIGHT) * 1.0 / 256;
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = dc_screen_offset.x + x;
  vert.y = dc_screen_offset.y + y + DC_PVR_FONT_HEIGHT;
  vert.z = z;
  vert.u = u1;
  vert.v = v2;
  vert.argb = argb;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + x;
  vert.y = dc_screen_offset.y + y;
  vert.u = u1;
  vert.v = v1;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + x + DC_PVR_FONT_WIDTH;
  vert.y = dc_screen_offset.y + y + DC_PVR_FONT_HEIGHT;
  vert.u = u2;
  vert.v = v2;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = dc_screen_offset.x + x + DC_PVR_FONT_WIDTH;
  vert.y = dc_screen_offset.y + y;
  vert.u = u2;
  vert.v = v1;
  pvr_prim (&vert, sizeof(vert));
}


void
dc_pvr_font_commit_string (float x, float y, float z, uint32 argb, const char *str)
{
  if (!str) return;
  
  while (*str)
  {
    dc_pvr_font_commit_char (x, y, z, argb, *str);
    
    x += 12;
    ++str;
  }
}


void
dc_pvr_font_output_message (const char *title, const char *message, const char *foot, dc_pvr_bgfunc_t bgfunc)
{
  int i;
  int y;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 w_color = 0xd0010101;
  
  /* fill PVR cache */
  for (i = 0; i < 2; ++i)
  {
    pvr_wait_ready ();
    pvr_scene_begin ();
    
    pvr_list_begin (PVR_LIST_OP_POLY);
    if (bgfunc) bgfunc();
    pvr_list_finish ();
    
    pvr_list_begin (PVR_LIST_TR_POLY);
    dc_pvr_font_commit_start ();
    if (title)
    {
      dc_pvr_font_commit_string (20, 50, 512, f_color, title);
      dc_pvr_font_commit_string (21, 51, 510, b_color, title);
    }
    
    if (message)
      dc_pvr_font_commit_string (50, 150, 512, f_color, message);
    
    if (foot) 
    {
      dc_pvr_font_commit_string (20, 400, 512, f_color, foot);
      dc_pvr_font_commit_string (21, 401, 510, b_color, foot);
    }
    
    dc_pvr_diagram_commit_start ();
    y = 80;
    dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color); 
    y = 430;
    dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color); 
    
    dc_pvr_diagram_commit_box (0, 0, 640, 480, 300, w_color);
    
    pvr_list_finish ();
    pvr_scene_finish ();
  }
}


/* ------------------------------------------------------------ */
pvr_poly_hdr_t dc_pvr_diagram_poly_hdr;


void
dc_pvr_diagram_init()
{
  pvr_poly_cxt_t poly;
  
  pvr_poly_cxt_col (&poly, PVR_LIST_TR_POLY);
  pvr_poly_compile(&dc_pvr_diagram_poly_hdr, &poly);
}


void
dc_pvr_diagram_commit_start ()
{
  pvr_prim(&dc_pvr_diagram_poly_hdr, sizeof(pvr_poly_hdr_t));
}


void
dc_pvr_diagram_commit_line(float x1, float y1, float x2, float y2, float z, uint32 argb)
{
  pvr_vertex_t vert;
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = dc_screen_offset.x + x1;
  vert.y = dc_screen_offset.y + y1 + 2;
  vert.z = z;
  vert.u = vert.v = 0.0f;
  vert.argb = argb;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + x1;
  vert.y = dc_screen_offset.y + y1;
  pvr_prim (&vert, sizeof(vert));

  vert.x = dc_screen_offset.x + x2;
  vert.y = dc_screen_offset.y + y2 + 2;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = dc_screen_offset.x + x2;
  vert.y = dc_screen_offset.y + y2;
  pvr_prim(&vert, sizeof(vert));
}


void
dc_pvr_diagram_commit_box (float x1, float y1, float x2, float y2, float z, uint32 argb)
{
  pvr_vertex_t vert;
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = dc_screen_offset.x + x1;
  vert.y = dc_screen_offset.y + y2;
  vert.z = z;
  vert.u = vert.v = 0.0f;
  vert.argb = argb;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + x1;
  vert.y = dc_screen_offset.y + y1;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + x2;
  vert.y = dc_screen_offset.y + y2;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = dc_screen_offset.x + x2;
  vert.y = dc_screen_offset.y + y1;
  pvr_prim(&vert, sizeof(vert));
}


/* ------------------------------------------------------------ */
static pvr_init_params_t pvr_params = 
{
  /* Enable opaque and translucent polygons with size 16 */
  { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0 },
  
  /* Vertex buffer size 512K */
  512*1024
};


void
dc_pvr_init ()
{
  pvr_init (&pvr_params); 
  
  dc_screen_offset_init ();
  dc_pvr_font_init ();
  dc_pvr_diagram_init ();
}


