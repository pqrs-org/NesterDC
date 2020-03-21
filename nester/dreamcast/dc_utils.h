#ifndef _DC_UTILS_H_
#define _DC_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#include "types.h"

extern const char *progname;


/* ------------------------------------------------------------ */
/* sq */

void dc_sq_cpy(void *dest, void *src, int n);
void dc_wait_sq_cpy_done ();


/* ------------------------------------------------------------ */
/* controller handling */

/* low level interface */
typedef enum {
  DC_MAPLE_CONTROLLER_NORMAL,
  DC_MAPLE_CONTROLLER_ARCADESTICK, 
} dc_maple_controller_type_t;

typedef enum {
  DC_MAPLE_MOUSE_3BUTTON,
} dc_maple_mouse_type_t;

typedef enum {
  DC_MAPLE_KEYBOARD_JP106,
} dc_maple_keyboard_type_t;

typedef struct {
  maple_device_t *dev;
  
  union {
    dc_maple_controller_type_t c_type;
    dc_maple_mouse_type_t m_type;
    dc_maple_keyboard_type_t k_type;
  } type;
} dc_maple_info_t;

#define DC_MAPLE_INFO_SIZE 16
extern dc_maple_info_t dc_maple_controller_info[DC_MAPLE_INFO_SIZE];
extern dc_maple_info_t dc_maple_mouse_info[DC_MAPLE_INFO_SIZE];
extern dc_maple_info_t dc_maple_keyboard_info[DC_MAPLE_INFO_SIZE];
void dc_maple_init ();

/* ------------------------------------------------------------ */
/* LCD handling */

void dc_lcd_icon_clear (uint8 *screen, int is_black);
void dc_lcd_icon_draw_char (uint8 *screen, int x, int y, int ch);
void dc_lcd_icon_draw_string (uint8 *screen, int x, int y, const char *str);
void dc_lcd_icon_flip(const uint8 *screen);
void dc_lcd_clear();


/* ------------------------------------------------------------ */
/* VMU utility */

maple_device_t *dc_vmu_search_file (const char *filename);


/* ------------------------------------------------------------ */
/* screen offset */

typedef struct {
  float x, y;
} dc_screen_offset_t;

extern dc_screen_offset_t dc_screen_offset;

void dc_screen_offset_init();


/* ------------------------------------------------------------ */
/* pvr handling */

void dc_pvr_init ();

typedef void (*dc_pvr_bgfunc_t)();

/* font */
void dc_pvr_font_commit_start ();
void dc_pvr_font_commit_string (float x, float y, float z, uint32 argb, const char *str);
void dc_pvr_font_output_message (const char *title, const char *message, const char *foot, dc_pvr_bgfunc_t bgfunc);

/* diagram */
void dc_pvr_diagram_commit_start ();
void dc_pvr_diagram_commit_line(float x1, float y1, float x2, float y2, float z, uint32 argb);
void dc_pvr_diagram_commit_box (float x1, float y1, float x2, float y2, float z, uint32 argb);


/* ------------------------------------------------------------ */
/* BMP handling */
int load_bmp (uint16 *raw, const char *filename);
void dc_bmpimage_copy_texture_320x240to512x256 (pvr_ptr_t txr_addr, uint16 *raw);
void dc_bmpimage_commit_texture_320x240on512x256 (pvr_poly_hdr_t *poly);


#ifdef __cplusplus
}
#endif

#endif

