#include <kos.h>
#include <string.h>
#include "nester.h"
#include "nes.h"
#include "null_sound_mgr.h"
#include "dc_main.h"
#include "dc_sound_mgr.h"
#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_controller.h"
#include "dc_menu_filelist.h"
#include "dc_menu_filelist_cdfs.h"
#include "dc_menu_filelist_nnnesterj_cheatcode.h"
#include "dc_menu_filelist_vmu.h"
#include "dc_menu_filelist_vmuselect.h"
#include "dc_menu_screenadjust.h"
#include "dc_menu_ingame.h"
#include "dc_saving.h"
#include "dc_vmu.h"
#include "fs_md.h"
#include "nes_pad.h"
#include "bzlib.h"


#define DC_MAIN_DRAW_FRAMESKIP 0

/* for bzip2 */
extern "C" void 
bz_internal_error (int errorcode) 
{
  char str[128];
  sprintf(str, "errorcode = 0x%x", errorcode);
  dc_pvr_font_output_message ("bzip2 Error", str, NULL, NULL);
  timer_spin_sleep (1000);
}

static uint16 startup_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 menu_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 credits_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 options_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 vmu_menu_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 menu_selection_image[320 * 240] __attribute__ ((aligned(32)));
static uint16 nes_screen[320 * 240] __attribute__ ((aligned(32)));

uint16
get_nesscreen_pixel_color(int x, int y)
{
  return nes_screen[y * 320 + x + 8];
}

static boolean exit_game_loop = false;
static boolean interrupt_game_loop = false;

static char last_romfile[256];
static maple_device_t *vmu_dev = NULL;


/* ------------------------------------------------------------ */
static const char *frameskip_string[] = {
  "NO SKIP",
  "AUTO: x1.0",
  "AUTO: x1.5", 
  "AUTO: x2.0", 
  "AUTO: x4.0",
};


static const int frameskip_cycles[] = {
  0,
  60,
  90,
  120,
  240,
};


dc_settings_t dc_settings;

static void
dc_settings_init()
{
  dc_settings.frameskip_rate = FRAMESKIP_x1;
  dc_settings.enable_sound = 1;
  dc_settings.enable_exsound = 1;
  dc_settings.enable_vmu = 1;
  dc_settings.enable_bilinear_filter = 1;
  
  dc_settings.enable_autofire = 0;
  dc_settings.enable_autofire_a = 1;
  dc_settings.enable_autofire_b = 1;
}


static const char *dc_settings_filename = "ndc_r7.1";
static const uint8 nesterdc_icon[] = {
#include "icons/nesterdc-icon.h"
};


static void
dc_settings_save(maple_device_t *dev)
{
  char *title = "Save User Settings";
  uint8 buf[512]; 
  uint8 *p;
  
  if (!dev) return;

#define DC_SETTING_SAVE_ITEM(x) { memcpy (p, &(x), sizeof(x)); p += 8; }
  
  p = buf;
  DC_SETTING_SAVE_ITEM(dc_settings.frameskip_rate);
  DC_SETTING_SAVE_ITEM(dc_settings.enable_sound);
  DC_SETTING_SAVE_ITEM(dc_settings.enable_exsound);
  DC_SETTING_SAVE_ITEM(dc_settings.enable_vmu);
  DC_SETTING_SAVE_ITEM(dc_settings.enable_bilinear_filter);
  
  DC_SETTING_SAVE_ITEM(dc_screen_offset.x);
  DC_SETTING_SAVE_ITEM(dc_screen_offset.y);
  
  DC_SETTING_SAVE_ITEM(dc_menu_filelist_global_config.cursor_wait);
  DC_SETTING_SAVE_ITEM(dc_menu_filelist_global_config.pageup_wait);
  
  dc_pvr_font_output_message (title, "Now Saving...", NULL, NULL);
  
  if (ndc_vmu_save(buf, sizeof(buf), dev, 
		   dc_settings_filename, "NesterDC Settings", "NesterDC Settings Compat7.1", nesterdc_icon) < 0)
  {
    dc_pvr_font_output_message (title, "Save Failed", NULL, NULL);
    timer_spin_sleep (1000);
  }
  else
  {
    dc_pvr_font_output_message (title, "Save Done", NULL, NULL);
  }
}


static void
dc_settings_load(maple_device_t *dev)
{
  char *title = "Load User Settings";
  uint8 buf[512];
  uint32 buf_len;
  
  dc_pvr_font_output_message (title, "Now Loading...", NULL, NULL);
  
  if (ndc_vmu_load(buf, &buf_len, dev, dc_settings_filename) < 0)
  {
    dc_pvr_font_output_message (title, "Load Failed", NULL, NULL);
    timer_spin_sleep (1000);
  }
  else
  {
    uint8 *p;
    
#define DC_SETTING_LOAD_ITEM(x) { memcpy(&(x), p, sizeof(x)); p += 8; }
    
    p = buf;
    DC_SETTING_LOAD_ITEM(dc_settings.frameskip_rate);
    DC_SETTING_LOAD_ITEM(dc_settings.enable_sound);
    DC_SETTING_LOAD_ITEM(dc_settings.enable_exsound);
    DC_SETTING_LOAD_ITEM(dc_settings.enable_vmu);
    DC_SETTING_LOAD_ITEM(dc_settings.enable_bilinear_filter);
    
    DC_SETTING_LOAD_ITEM(dc_screen_offset.x);
    DC_SETTING_LOAD_ITEM(dc_screen_offset.y);
    
    DC_SETTING_LOAD_ITEM(dc_menu_filelist_global_config.cursor_wait);
    DC_SETTING_LOAD_ITEM(dc_menu_filelist_global_config.pageup_wait);
    
    dc_pvr_font_output_message (title, "Load Done", NULL, NULL);
  }
}


/* ------------------------------------------------------------ */
/* DC_TEXTURE_ADDRS_SIZE must be 2^x */
#define NES_TEXTURE_ADDRS_SIZE 4
static pvr_ptr_t nes_texture_addrs[NES_TEXTURE_ADDRS_SIZE];
static pvr_poly_hdr_t nes_pvr_poly_headers_filter_none[NES_TEXTURE_ADDRS_SIZE];
static pvr_poly_hdr_t nes_pvr_poly_headers_filter_bilinear[NES_TEXTURE_ADDRS_SIZE];

static pvr_ptr_t dc_texture_addr;
static pvr_poly_hdr_t dc_pvr_poly_header;


static void
texture_init ()
{
  int i;
  pvr_poly_cxt_t poly;
  pvr_ptr_t addr;
  
  addr = pvr_mem_malloc (512 * 256 * 2);
  pvr_poly_cxt_txr (&poly, PVR_LIST_OP_POLY, 
                    PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
                    512, 256, addr, PVR_FILTER_NONE);
  dc_texture_addr = addr;
  pvr_poly_compile (&dc_pvr_poly_header, &poly);
  
  for (i = 0; i < NES_TEXTURE_ADDRS_SIZE; ++i) 
  {
    addr = pvr_mem_malloc (256 * 256 * 2);
    
    nes_texture_addrs[i] = addr; 
    
    pvr_poly_cxt_txr (&poly, PVR_LIST_OP_POLY, 
                      PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
                      256, 256, addr, PVR_FILTER_NONE);
    pvr_poly_compile (nes_pvr_poly_headers_filter_none + i, &poly);
    
    pvr_poly_cxt_txr (&poly, PVR_LIST_OP_POLY, 
                      PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
                      256, 256, addr, PVR_FILTER_BILINEAR);
    pvr_poly_compile (nes_pvr_poly_headers_filter_bilinear + i, &poly);
  }
}


#if DC_MAIN_DRAW_FRAMESKIP
int skip_frames = 0;
#endif

static void
dc_nesscreen_copy_texture_320x240to256x256 (pvr_ptr_t txr_addr)
{
  int h;
  uint16 *d = (uint16 *)(txr_addr);
  uint16 *s = nes_screen + 8;
  
  for (h = 0; h < 224; ++h)
  {
    dc_sq_cpy (d, s, 256 * 2);
    d += 256;
    s += 320;
  }
  dc_wait_sq_cpy_done ();
}


static void
dc_nesscreen_commit_texture_320x240on256x256 (pvr_poly_hdr_t *poly)
{
  pvr_vertex_t vert;
  
  pvr_prim (poly, sizeof(pvr_poly_hdr_t));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = dc_screen_offset.x;
  vert.y = dc_screen_offset.y + 224 * 2;
  vert.z = 256;
  vert.u = 0;
  vert.v = 224.0/256;
  vert.argb = 0xffffffff;
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x;
  vert.y = dc_screen_offset.y;
  vert.u = 0;
  vert.v = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = dc_screen_offset.x + 640;
  vert.y = dc_screen_offset.y + 224 * 2;
  vert.u = 1.0;
  vert.v = 224.0/256;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = dc_screen_offset.x + 640;
  vert.y = dc_screen_offset.y;
  vert.u = 1.0;
  vert.v = 0;
  pvr_prim (&vert, sizeof(vert));
}


void
dc_menu_ingame_init ()
{
  dc_nesscreen_copy_texture_320x240to256x256 (nes_texture_addrs[0]);
}


void
dc_menu_ingame_bgfunc ()
{
  dc_nesscreen_commit_texture_320x240on256x256 (nes_pvr_poly_headers_filter_none);
}


static void
display_nes_screen (NES *emu)
{
  static int texture_index = 0;
  uint32 f_color = 0xffffffff;
  
  dc_nesscreen_copy_texture_320x240to256x256 (nes_texture_addrs[texture_index]);
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (dc_settings.enable_bilinear_filter)
    dc_nesscreen_commit_texture_320x240on256x256 (nes_pvr_poly_headers_filter_bilinear + texture_index);
  else
    dc_nesscreen_commit_texture_320x240on256x256 (nes_pvr_poly_headers_filter_none + texture_index);
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  if (emu->ex_controller_type == NES::EX_OPTICAL_GUN ||
      emu->ex_controller_type == NES::EX_SPACE_SHADOW_GUN)
  {
    float x, y;
    dc_pvr_font_commit_start ();
    x = emu->NES_gun.x * 640.0/256 - 6;
    y = emu->NES_gun.y * 2 - 12;
    dc_pvr_font_commit_string (x, y, 512, f_color, "+");
  }
#if DC_MAIN_DRAW_FRAMESKIP
  dc_pvr_font_commit_start ();
  char str[256];
  sprintf(str, "skip_frames = %d", skip_frames);
  dc_pvr_font_commit_string (30, 30, 512, f_color, str);
  dc_pvr_font_commit_string (32, 32, 511, 0xff010101, str);
#endif
  pvr_list_finish ();
  pvr_scene_finish ();
  
  texture_index = (texture_index + 1) & (NES_TEXTURE_ADDRS_SIZE - 1);
}


/* ------------------------------------------------------------ */
static void 
toupper_string (char *str)
{
  char *p = str;
  while (*p)
  {
    *p = toupper(*p);
    ++p;
  }
}


static void 
vmu_draw_settings (NES *emu)
{
  uint8 vmu_screen[48 * 32];
  char str[256];
  
  dc_lcd_icon_clear (vmu_screen, 1);
  
  dc_lcd_icon_draw_string (vmu_screen, 1, 1, "FRAME RATE:");
  dc_lcd_icon_draw_string (vmu_screen, 3, 7, 
			   frameskip_string[dc_settings.frameskip_rate]);
  
  sprintf(str, "AUTOFIRE: %s", dc_settings.enable_autofire ? "Y" : "N");
  dc_lcd_icon_draw_string (vmu_screen, 1, 13, str);
  
  sprintf(str, "%s", emu->getROMname ());
  toupper_string (str);
  dc_lcd_icon_draw_string (vmu_screen, 1, 26, str);
  
  dc_lcd_icon_flip (vmu_screen);
}


/* ============================================================ */
static uint8 keymap_jp106_dc2nes[256] = {
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  NES_device_kbd::NES_KEY_VoidSymbol,
  
  NES_device_kbd::NES_KEY_A,	 /* 0x04 */
  NES_device_kbd::NES_KEY_B,
  NES_device_kbd::NES_KEY_C,
  NES_device_kbd::NES_KEY_D,
  NES_device_kbd::NES_KEY_E,
  NES_device_kbd::NES_KEY_F,
  NES_device_kbd::NES_KEY_G,
  NES_device_kbd::NES_KEY_H,
  NES_device_kbd::NES_KEY_I,
  NES_device_kbd::NES_KEY_J,
  NES_device_kbd::NES_KEY_K,
  NES_device_kbd::NES_KEY_L,
  NES_device_kbd::NES_KEY_M,
  NES_device_kbd::NES_KEY_N,
  NES_device_kbd::NES_KEY_O,
  NES_device_kbd::NES_KEY_P,
  NES_device_kbd::NES_KEY_Q,
  NES_device_kbd::NES_KEY_R,
  NES_device_kbd::NES_KEY_S,
  NES_device_kbd::NES_KEY_T,
  NES_device_kbd::NES_KEY_U,
  NES_device_kbd::NES_KEY_V,
  NES_device_kbd::NES_KEY_W,
  NES_device_kbd::NES_KEY_X,
  NES_device_kbd::NES_KEY_Y,
  NES_device_kbd::NES_KEY_Z,
  
  NES_device_kbd::NES_KEY_1,	/* 0x1e */
  NES_device_kbd::NES_KEY_2,
  NES_device_kbd::NES_KEY_3,
  NES_device_kbd::NES_KEY_4,
  NES_device_kbd::NES_KEY_5,
  NES_device_kbd::NES_KEY_6,
  NES_device_kbd::NES_KEY_7,
  NES_device_kbd::NES_KEY_8,
  NES_device_kbd::NES_KEY_9,
  NES_device_kbd::NES_KEY_0,
  
  NES_device_kbd::NES_KEY_Return,	/* 0x28 */
  NES_device_kbd::NES_KEY_Escape,
  NES_device_kbd::NES_KEY_Delete,	/* BACKSPACE */
  NES_device_kbd::NES_KEY_VoidSymbol, 	/* TAB */
  NES_device_kbd::NES_KEY_Space,
  NES_device_kbd::NES_KEY_minus,
  NES_device_kbd::NES_KEY_caret,	/* PLUS (CARET on jp106) */ 
  NES_device_kbd::NES_KEY_at,		/* LBRACKET (AT on jp106) */
  NES_device_kbd::NES_KEY_bracketleft,	/* RBRACKET (LBRACKET on jp106) */
  NES_device_kbd::NES_KEY_VoidSymbol,	/* BACKSLASH (UNKNOWN on jp106) */
  NES_device_kbd::NES_KEY_bracketright, /* UNKNOWN (RBACKET on jp106) */
  NES_device_kbd::NES_KEY_semicolon,
  NES_device_kbd::NES_KEY_colon,	/* QUOTE (COLON on jp106) */
  NES_device_kbd::NES_KEY_VoidSymbol,   /* TILDE (UNKNOWN on jp106) */
  
  NES_device_kbd::NES_KEY_comma,	/* 0x36 */
  NES_device_kbd::NES_KEY_period, 
  NES_device_kbd::NES_KEY_slash,
  NES_device_kbd::NES_KEY_Control_L,	/* CAPSLOCK */
  
  NES_device_kbd::NES_KEY_F1,	/* 0x3a */
  NES_device_kbd::NES_KEY_F2,
  NES_device_kbd::NES_KEY_F3,
  NES_device_kbd::NES_KEY_F4,
  NES_device_kbd::NES_KEY_F5,
  NES_device_kbd::NES_KEY_F6,
  NES_device_kbd::NES_KEY_F7,
  NES_device_kbd::NES_KEY_F8,
  NES_device_kbd::NES_KEY_VoidSymbol,	/* F9 */
  NES_device_kbd::NES_KEY_VoidSymbol,	/* F10 */
  NES_device_kbd::NES_KEY_VoidSymbol,	/* F11 */
  NES_device_kbd::NES_KEY_Kana,
  
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PRINT */ /* 0x46 */
  NES_device_kbd::NES_KEY_VoidSymbol,	/* SCRLOCK */
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PAUSE */
  NES_device_kbd::NES_KEY_Insert,
  NES_device_kbd::NES_KEY_Clear,
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PGUP */
  NES_device_kbd::NES_KEY_Delete,
  NES_device_kbd::NES_KEY_Stop,
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PGDOWN */
  NES_device_kbd::NES_KEY_Right,
  NES_device_kbd::NES_KEY_Left,
  NES_device_kbd::NES_KEY_Down,
  NES_device_kbd::NES_KEY_Up,
  
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PAD_NUMLOCK */ /* 0x53 */
  NES_device_kbd::NES_KEY_slash,
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PAD_MULTIPLY */
  NES_device_kbd::NES_KEY_minus,
  NES_device_kbd::NES_KEY_VoidSymbol,	/* PAD_PLUS */
  NES_device_kbd::NES_KEY_Return,
  NES_device_kbd::NES_KEY_1,
  NES_device_kbd::NES_KEY_2,
  NES_device_kbd::NES_KEY_3,
  NES_device_kbd::NES_KEY_4,
  NES_device_kbd::NES_KEY_5,
  NES_device_kbd::NES_KEY_6,
  NES_device_kbd::NES_KEY_7,
  NES_device_kbd::NES_KEY_8,
  NES_device_kbd::NES_KEY_9,
  NES_device_kbd::NES_KEY_0,
  NES_device_kbd::NES_KEY_period,
  NES_device_kbd::NES_KEY_VoidSymbol, /* S3 */
};

static uint8 *keymap_dc2nes;

static void
keymap_dc2nes_init()
{
  keymap_jp106_dc2nes[0x87] = NES_device_kbd::NES_KEY_apostrophe;
  keymap_jp106_dc2nes[0x89] = NES_device_kbd::NES_KEY_backslash;
  
  keymap_dc2nes = keymap_jp106_dc2nes;
}


static void
poll_input_optional_gun (NES *emu)
{
  cont_state_t *stat;
  mouse_state_t *mstat;
  
  emu->NES_gun.buttons = 0;
  
  stat = (cont_state_t *)maple_dev_status(dc_maple_controller_info->dev);
  
  if (stat->buttons & CONT_DPAD_UP) emu->NES_gun.y -= 2;
  if (stat->buttons & CONT_DPAD_DOWN) emu->NES_gun.y += 2;
  if (stat->buttons & CONT_DPAD_LEFT) emu->NES_gun.x -= 2;
  if (stat->buttons & CONT_DPAD_RIGHT) emu->NES_gun.x += 2;
  if (stat->buttons & CONT_X) emu->NES_gun.buttons |= NES_device_gun::BUTTON_L;
  if (stat->buttons & CONT_A) emu->NES_gun.buttons |= NES_device_gun::BUTTON_R;
  
  if (dc_maple_mouse_info->dev)
  {
    mstat = (mouse_state_t *)maple_dev_status (dc_maple_mouse_info->dev);
    
    emu->NES_gun.x += mstat->dx;
    emu->NES_gun.y += mstat->dy;
    if (mstat->buttons & MOUSE_LEFTBUTTON) emu->NES_gun.buttons |= NES_device_gun::BUTTON_L;
    if (mstat->buttons & MOUSE_RIGHTBUTTON) emu->NES_gun.buttons |= NES_device_gun::BUTTON_R;
  }
  emu->NES_gun.normalize_position();
}


static void
poll_input_family_keyboard (NES *emu) 
{
  emu->NES_kbd.clear_register ();
  
  if (dc_maple_keyboard_info->dev)
  {
    kbd_state_t *kstat;
    kbd_cond_t *cond;
    int i;
    
    kstat = (kbd_state_t *)maple_dev_status (dc_maple_keyboard_info->dev);
    cond = &(kstat->cond);
    
    if (cond->modifiers & KBD_MOD_LCTRL) emu->NES_kbd.setkey(NES_device_kbd::NES_KEY_Control_L);
    if (cond->modifiers & KBD_MOD_LSHIFT) emu->NES_kbd.setkey(NES_device_kbd::NES_KEY_Shift_L);
    if (cond->modifiers & KBD_MOD_RSHIFT) emu->NES_kbd.setkey(NES_device_kbd::NES_KEY_Shift_R);
    if (cond->modifiers & KBD_MOD_LALT) emu->NES_kbd.setkey(NES_device_kbd::NES_KEY_Grph);
    
    for (i = 0; i < sizeof(cond->keys)/sizeof(cond->keys[0]); ++i)
    {
      if (cond->keys[i]) emu->NES_kbd.setkey(keymap_dc2nes[cond->keys[i]]);
    }
  }
}


static void
poll_input_cclimber (NES *emu)
{
  cont_state_t *stat;
  
  stat = (cont_state_t *)maple_dev_status(dc_maple_controller_info->dev);
  
  emu->NES_cclimber.left_UP = stat->buttons & CONT_DPAD_UP;
  emu->NES_cclimber.left_DOWN = stat->buttons & CONT_DPAD_DOWN;
  emu->NES_cclimber.left_LEFT = stat->buttons & CONT_DPAD_LEFT;
  emu->NES_cclimber.left_RIGHT = stat->buttons & CONT_DPAD_RIGHT;
  
  emu->NES_cclimber.right_UP = stat->buttons & CONT_Y;
  emu->NES_cclimber.right_DOWN = stat->buttons & CONT_A;
  emu->NES_cclimber.right_LEFT = stat->buttons & CONT_X;
  emu->NES_cclimber.right_RIGHT = stat->buttons & CONT_B;
}


static void
poll_input_paddle (NES *emu)
{
  mouse_state_t *mstat;
  
  emu->NES_paddle.buttons = 0;
  
  if (dc_maple_mouse_info->dev)
  {
    mstat = (mouse_state_t *)maple_dev_status (dc_maple_mouse_info->dev);
    
    emu->NES_paddle.x += mstat->dx;
    if (mstat->buttons & MOUSE_LEFTBUTTON) emu->NES_paddle.buttons |= NES_device_paddle::BUTTON_L;
    if (mstat->buttons & MOUSE_RIGHTBUTTON) emu->NES_paddle.buttons |= NES_device_paddle::BUTTON_R;
  }
  emu->NES_paddle.normalize_position();
}


static void
poll_input_mahjong (NES *emu) 
{
  emu->NES_kbd.clear_register ();
  
  if (dc_maple_keyboard_info->dev)
  {
    kbd_state_t *kstat;
    
    kstat = (kbd_state_t *)maple_dev_status (dc_maple_keyboard_info->dev);
    
    emu->NES_mahjong.setkey(kstat->matrix);
  }
}


static void 
poll_input(NES* emu) 
{
  int n;
  dc_maple_info_t *p;
  cont_state_t *stat;
  static int autofire_counter = 0;
  NES_pad *pad;
  uint16 buttons;
  
  autofire_counter ^= 1;
  
  n = 0;
  p = dc_maple_controller_info;
  while (p->dev)
  {
    stat = (cont_state_t *)maple_dev_status (p->dev);
    
    if (stat->joyy < -120) /* analog UP */
    {
      interrupt_game_loop = true;
      dc_settings.frameskip_rate = FRAMESKIP_NONE;
      return;
    }
    else if (stat->joyy > 120) /* analog DOWN */
    {
      interrupt_game_loop = true;
      dc_settings.frameskip_rate = FRAMESKIP_x2;
      return;
    }
    else if (stat->joyx < -120) /* analog LEFT */
    {
      interrupt_game_loop = true;
      dc_settings.frameskip_rate = (stat->rtrig > 128) ? FRAMESKIP_x4 : FRAMESKIP_x1_5;
      return;
    }
    else if (stat->joyx > 120) /* analog RIGHT */
    {
      interrupt_game_loop = true;
      dc_settings.frameskip_rate = FRAMESKIP_x1;
      return;
    }
    
    if (stat->rtrig > 128) 
    {
      if ((stat->ltrig > 128) && (stat->buttons & CONT_START))
      {
	exit_game_loop = true;
	interrupt_game_loop = true;
	return;
      }
      else if (stat->buttons & CONT_B)
      {
	dc_menu_ingame_statesave (emu, DC_MENU_INGAME_STATESAVE_QUICK, vmu_dev);
	interrupt_game_loop = true;
	return;
      }
      else if (stat->buttons & CONT_Y)
      {
	dc_menu_ingame_statesave (emu, DC_MENU_INGAME_STATESAVE_VMU, vmu_dev);
	interrupt_game_loop = true;
	return;
      }
      else if (stat->buttons & CONT_X)
      {
	dc_menu_ingame_system (emu);
	interrupt_game_loop = true;
	return;
      }
      else if (stat->buttons & CONT_A)
      {
	dc_menu_ingame_option (emu);
	interrupt_game_loop = true;
	return;
      }
      else if (stat->buttons & CONT_START)
      {
        dc_menu_ingame_cheat (emu);
        interrupt_game_loop = true;
        return;
      }
    }
    
    /* ------------------------------------------------------------ */
    /* NesterDC specific */
    pad = emu->get_pad (n & 0x03);
    buttons = stat->buttons;
    
    if (p->type.c_type == DC_MAPLE_CONTROLLER_ARCADESTICK)
    {
      static int c_counter[DC_MAPLE_INFO_SIZE];
      
      if (stat->buttons & CONT_C)
        ++(c_counter[n]);
      else
        c_counter[n] = 0;
      
      if (c_counter[n] == 1) 
      {
        dc_settings.enable_autofire = !(dc_settings.enable_autofire);
        interrupt_game_loop = true;
        return;
      }
      
      if (dc_settings.enable_autofire && autofire_counter)
      {
	if (dc_settings.enable_autofire_b && (stat->buttons & CONT_X))
	  buttons &= ~CONT_X;
	if (dc_settings.enable_autofire_a && (stat->buttons & CONT_Y))
	  buttons &= ~CONT_Y;
      }
      
      pad->nes_A = buttons & CONT_Y;
      pad->nes_B = buttons & CONT_X;
      pad->nes_SELECT = buttons & CONT_Z;
      pad->nes_START = buttons & (CONT_START | CONT_A);
    }
    else
    {
      static int ltrig_counter[DC_MAPLE_INFO_SIZE];
      
      if (stat->ltrig > 128)
        ++(ltrig_counter[n]);
      else
        ltrig_counter[n] = 0;
      
      if (ltrig_counter[n] == 1) 
      {
        dc_settings.enable_autofire = !(dc_settings.enable_autofire);
        interrupt_game_loop = true;
        return;
      }
      
      if (dc_settings.enable_autofire && autofire_counter)
      {
	if (dc_settings.enable_autofire_b && (stat->buttons & CONT_X))
	  buttons &= ~CONT_X;
	if (dc_settings.enable_autofire_a && (stat->buttons & CONT_A))
	  buttons &= ~CONT_A;
      }
      
      pad->nes_A = buttons & CONT_A;
      pad->nes_B = buttons & CONT_X;
      pad->nes_SELECT = buttons & CONT_Y;
      pad->nes_START = buttons & CONT_START;
    }
    
    pad->nes_UP = buttons & CONT_DPAD_UP;
    pad->nes_DOWN = buttons & CONT_DPAD_DOWN;
    pad->nes_LEFT = buttons & CONT_DPAD_LEFT;
    pad->nes_RIGHT = buttons & CONT_DPAD_RIGHT;
    
    ++p;
    ++n;
  }
  
  if (emu->ex_controller_type != NES::EX_NONE)
  {
    switch (emu->ex_controller_type) {
      case NES::EX_OPTICAL_GUN:
      case NES::EX_SPACE_SHADOW_GUN:
        poll_input_optional_gun (emu);
        break;
        
      case NES::EX_FAMILY_KEYBOARD:
        poll_input_family_keyboard (emu);
        break;
        
      case NES::EX_CRAZY_CLIMBER:
        poll_input_cclimber (emu);
        break;
        
      case NES::EX_ARKANOID_PADDLE: 
	poll_input_paddle (emu);
	break;
        
      case NES::EX_MAHJONG: 
	poll_input_mahjong (emu);
	break;
    }
  }
}


/* ------------------------------------------------------------ */
static void 
display_menu_main_option (pvr_poly_hdr_t *bg_poly_hdr)
{
  int y;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (bg_poly_hdr) dc_bmpimage_commit_texture_320x240on512x256 (bg_poly_hdr);
  pvr_list_finish ();
  
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  
  dc_pvr_font_commit_string (20, 50, 512, f_color, "Options");
  dc_pvr_font_commit_string (21, 51, 510, b_color, "Options");
  
  y = 150;
  dc_pvr_font_commit_string (50, y, 500, f_color, "UP:    Enable/Disable Sound");
  dc_pvr_font_commit_string (500, y, 500, f_color, dc_settings.enable_sound ? "[Enable]" : "[Disable]");
  y += 24; 
  dc_pvr_font_commit_string (50, y, 500, f_color, "DOWN:  Enable/Disable VMU");
  dc_pvr_font_commit_string (500, y, 500, f_color, dc_settings.enable_vmu ? "[Enable]" : "[Disable]");
  y += 24; 
  dc_pvr_font_commit_string (50, y, 500, f_color, "LEFT:  Enable/Disable ExSound");
  dc_pvr_font_commit_string (500, y, 500, f_color, dc_settings.enable_exsound ? "[Enable]" : "[Disable]");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "RIGHT: Enable/Disable Bilinear Filter");
  dc_pvr_font_commit_string (500, y, 500, f_color, dc_settings.enable_bilinear_filter ? "[Enable]" : "[Disable]");
  y += 24; 
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "Y:     Adjust Screen Position");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "A:     Controller Setting Menu");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "R + B: Set Default");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "X:     Save To VMU");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "L + R: Exit");
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


static void
dc_menu_main_option ()
{
  dc_menu_controller_status_t *p;
  
  dc_menu_controller_status_init();
  dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, options_image);
  
  for (;;) 
  {
    dc_menu_controller_status_scan();
    display_menu_main_option (&dc_pvr_poly_header);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->ltrig))
        return;
      
#define DC_MENU_MAIN_OPTION_TOGGLE_SETTING(KEY,VALUE) { \
      if (dc_menu_keypress(p->KEY)) \
	VALUE = !(VALUE); \
}
      
      DC_MENU_MAIN_OPTION_TOGGLE_SETTING(DPAD_UP, dc_settings.enable_sound);
      DC_MENU_MAIN_OPTION_TOGGLE_SETTING(DPAD_DOWN, dc_settings.enable_vmu);
      DC_MENU_MAIN_OPTION_TOGGLE_SETTING(DPAD_LEFT, dc_settings.enable_exsound);	
      DC_MENU_MAIN_OPTION_TOGGLE_SETTING(DPAD_RIGHT, dc_settings.enable_bilinear_filter);
      
      if (dc_menu_keypress(p->A))
      {
	dc_menu_controller ();
        dc_menu_controller_status_init ();
      }
      
      if (dc_menu_keypress(p->Y))
      {
	dc_menu_screenadjust (NULL);
        dc_menu_controller_status_init ();
      }
      
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->B))
	dc_settings_init ();
      
      if (dc_menu_keypress(p->X))
	dc_settings_save (vmu_dev);
      
      ++p;
    }
  }
}


/* ============================================================ */
static void
display_credits()
{
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  int y;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  dc_bmpimage_commit_texture_320x240on512x256 (&dc_pvr_poly_header);
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  dc_pvr_font_commit_string (430, 440, 512, f_color, "Press R+L");
  dc_pvr_font_commit_string (431, 441, 510, b_color, "Press R+L");
  
  dc_pvr_diagram_commit_start ();
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  pvr_list_finish ();
  
  pvr_scene_finish ();
}

static void 
do_credits() 
{
  dc_menu_controller_status_t *p;
  
  dc_menu_controller_status_init ();
  dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, credits_image);
  
  for (;;)
  {
    dc_menu_controller_status_scan ();
    display_credits ();
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->ltrig))
	return;
      
      ++p;
    }
  }
}


/* ============================================================ */
static volatile int rest_frames = 0;

static void
timer_handler(irq_t source, irq_context_t *context)
{
  ++rest_frames;
}


static void
do_autoloop (NES *emu, int cycles_per_sec)
{
  timer_prime (TMU2, cycles_per_sec, true);
  irq_set_handler (EXC_TMU2_TUNI2, timer_handler);
  timer_start (TMU2);
  
#if DC_MAIN_DRAW_FRAMESKIP
  skip_frames = 0;
#endif
  rest_frames = 1;
  for (;;)
  {
    emu->emulate_frame (nes_screen);
    poll_input (emu);
    
    display_nes_screen (emu);
    if (interrupt_game_loop) break;
    
    if (--rest_frames > 0) 
    {
      int drop_frames = rest_frames;
      
      rest_frames = 0;
      while (drop_frames--)
      {
	emu->emulate_frame_skip ();
	poll_input (emu);
	if (interrupt_game_loop)
	  break;
#if DC_MAIN_DRAW_FRAMESKIP
        ++skip_frames;
#endif
      }
    }
  }
  timer_stop (TMU2);
}


static void
do_loop (NES *emu)
{
  exit_game_loop = false;
  interrupt_game_loop = false;
  
  while (!exit_game_loop) 
  {
    emu->freeze ();
    vmu_draw_settings (emu);
    emu->thaw ();
    interrupt_game_loop = false;
    
    if (dc_settings.frameskip_rate == FRAMESKIP_NONE)
    {
      while (!interrupt_game_loop) 
      {
	emu->emulate_frame (nes_screen);
	poll_input(emu);
	display_nes_screen (emu);
      }
    }
    else
    {
      do_autoloop (emu, frameskip_cycles[dc_settings.frameskip_rate]);
    }
  }
}


static void
run_game(const char *filename)
{
  NES *emu = NULL;
  sound_mgr *snd_mgr = NULL;
  
  dc_pvr_font_output_message ("Initialize", "Wait A Moment", NULL, NULL);
  
  if (dc_settings.enable_sound)
    snd_mgr = new dc_sound_mgr ();
  else
    snd_mgr = new null_sound_mgr ();
  if (!snd_mgr) goto finish;
  if (!(snd_mgr->initialize ())) goto finish;
  
  emu = new NES (snd_mgr);
  if (!emu) goto finish;
  sprintf(emu->disksys_rom_filename, "/cd/games/disksys.rom");
  if (!(emu->initialize (filename))) goto finish;
  
  if (dc_settings.enable_vmu)
  {
    sram_load_from_vmu (vmu_dev, emu);
    disk_load_from_vmu (vmu_dev, emu);
  }
  
  dc_settings.frameskip_rate = FRAMESKIP_x1;
  dc_settings.enable_autofire = 0;
  
  do_loop (emu);
  
  emu->freeze ();
  
  dc_pvr_font_output_message ("Terminate", "Wait A Moment", NULL, NULL);
  
  if (dc_settings.enable_vmu)
  {
    sram_save_to_vmu (vmu_dev, emu);
    disk_save_to_vmu (vmu_dev, emu);
  }
  emu->thaw ();
  
finish:
  if (emu) delete (emu);
  if (snd_mgr) delete (snd_mgr);
}


static void
md_clear ()
{
  FILE *fp;
  
  fp = fopen("/md/quick", "w");
  if (fp)
    fclose(fp);
}


void
dc_menu_listing_cdfs_bgfunc()
{
  dc_bmpimage_commit_texture_320x240on512x256 (&dc_pvr_poly_header);
}


static void
do_rom_menu_and_run () 
{
  char *p;
  char result[256];
  char last_romfile_dir[256];
  
  sprintf (last_romfile_dir, "%s", last_romfile);
  p = strrchr(last_romfile_dir, '/');
  *p = '\0';
  
  dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, menu_selection_image);
  
  dc_menu_filelist_cdfs (result, sizeof(result), last_romfile_dir, dc_menu_listing_cdfs_bgfunc);
  
  if (*result)
  {
    if (strcmp(result, last_romfile))
      md_clear ();
    
    run_game (result);
    sprintf (last_romfile, "%s", result);
  }
}


/* ============================================================ */
static void
display_menu_main_menu (pvr_poly_hdr_t *bg_poly_hdr)
{
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (bg_poly_hdr) dc_bmpimage_commit_texture_320x240on512x256 (bg_poly_hdr);
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  pvr_list_finish ();
  
  pvr_scene_finish ();
}


static void
draw_main_menu_vmu_icon ()
{
  uint8 vmu_screen[48 * 32];
  int vmu_free_blocks;
  char str[32];
  
  vmu_free_blocks = ndc_vmu_check_free_blocks (NULL, vmu_dev);
  
  dc_lcd_icon_clear (vmu_screen, 1);
  dc_lcd_icon_draw_string (vmu_screen, 1, 1, "VMU FREE:");
  sprintf(str, "%d BLOCKS", vmu_free_blocks);
  dc_lcd_icon_draw_string (vmu_screen, 3, 7, str);
  
  dc_lcd_icon_draw_string (vmu_screen, 1, 20, "NESTERDC");
  dc_lcd_icon_draw_string (vmu_screen, 3, 26, "7.1 RELEASE");
  
  dc_lcd_icon_flip (vmu_screen);
}


/* ------------------------------------------------------------ */
static void
dc_menu_filelist_vmu_bgfunc()
{
  dc_bmpimage_commit_texture_320x240on512x256 (&dc_pvr_poly_header);
}


static void
dc_menu_main_menu ()
{
  dc_menu_controller_status_t *p;
  
start:
  
  dc_menu_controller_status_init ();
  dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, menu_image);
  draw_main_menu_vmu_icon ();
  
  for (;;) 
  {
    dc_menu_controller_status_scan ();
    display_menu_main_menu (&dc_pvr_poly_header);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->ltrig) &&
          dc_menu_keypressing(p->Y) && dc_menu_keypressing(p->START))
	return;
      
      if (dc_menu_keypress(p->A))
      {
	do_rom_menu_and_run ();
        goto start;
      }
      
      if (dc_menu_keypress(p->X))
      {
	dc_menu_main_option ();
        goto start;
      }
      
      if (dc_menu_keypress(p->Y))
      {
	do_credits ();
        goto start;
      }
      
      if (dc_menu_keypress(p->B))
      {
	dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, vmu_menu_image);
        dc_menu_filelist_vmu (vmu_dev, dc_menu_filelist_vmu_bgfunc);
	
        goto start;
      }
      ++p;
    }
  }
}


/* ============================================================ */
typedef struct {
  const char *filename;
  uint16 *buf;
} bmpimage_fileinfo_t;


void 
dc_main_loadbmp_bgfunc ()
{
  dc_bmpimage_commit_texture_320x240on512x256 (&dc_pvr_poly_header);
}


static void
dc_main_loadbmp ()
{
  bmpimage_fileinfo_t bmpimage_fileinfo[] = {
    { "/cd/pics/menu.bmp", menu_image },
    { "/cd/pics/credits.bmp", credits_image },
    { "/cd/pics/options.bmp", options_image },
    { "/cd/pics/vmu_menu.bmp", vmu_menu_image },
    { "/cd/pics/menu_selection.bmp", menu_selection_image },
  };
  bmpimage_fileinfo_t *p;
  int i;
  const char *nesterdc_version = "NesterDC 7.1";
  
  dc_pvr_font_output_message (nesterdc_version, NULL, "loading bmp-images", NULL);
  load_bmp (startup_image, "/cd/pics/startup.bmp");
  dc_bmpimage_copy_texture_320x240to512x256 (dc_texture_addr, startup_image);
  
  p = bmpimage_fileinfo;
  for (i = 0; i < sizeof(bmpimage_fileinfo)/sizeof(bmpimage_fileinfo[0]); ++i)
  {
    dc_pvr_font_output_message (nesterdc_version, NULL, p->filename, dc_main_loadbmp_bgfunc);
    load_bmp (p->buf, p->filename);
    ++p;
  }
}


/* ============================================================ */
#if 0
#include "testing.cpp"
#endif


const char *progname = "***NesterDC***";
KOS_INIT_FLAGS(INIT_IRQ);

extern "C" int
main() 
{
#ifdef __DC_PAL__
  vid_init (DM_640x480_PAL_IL, PM_RGB565);
#endif
  dc_pvr_init ();
  texture_init ();
  dc_settings_init ();
  keymap_dc2nes_init ();
  
  fs_md_init (NES::snss_buflen);
  dc_lcd_clear ();
  
  dc_menu_filelist_init ();
  dc_menu_controller_init ();
  
  dc_menu_controller ();
  
#if 0
  test_all ();
#endif
  
  dc_main_loadbmp();
  
  vmu_dev = dc_vmu_search_file (dc_settings_filename);
  if (!vmu_dev) 
  {
    vmu_dev = dc_menu_filelist_vmuselect ();
    if (vmu_dev) dc_settings_save (vmu_dev);
  }
  if (vmu_dev) dc_settings_load (vmu_dev);
  
  sprintf (last_romfile, "/cd/games/game.nes");
  
  run_game ("/cd/games/game.nes");
  dc_menu_main_menu();
  
  dc_lcd_clear ();
  
  fs_md_shutdown ();
  return 0;
}


