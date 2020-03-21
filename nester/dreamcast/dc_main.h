#ifndef _DC_MAIN_H_
#define _DC_MAIN_H_


typedef enum
{
  FRAMESKIP_NONE,
  FRAMESKIP_x1,
  FRAMESKIP_x1_5,
  FRAMESKIP_x2,
  FRAMESKIP_x4,
} frameskip_rate_t;


typedef struct {
  frameskip_rate_t frameskip_rate;
  uint8	enable_sound;
  uint8 enable_exsound;
  uint8	enable_vmu;
  uint8 enable_bilinear_filter;
  
  uint8 enable_autofire;
  uint8 enable_autofire_a;
  uint8 enable_autofire_b;
} dc_settings_t;

extern dc_settings_t dc_settings;

extern void dc_menu_ingame_init();
extern void dc_menu_ingame_bgfunc();

#endif
