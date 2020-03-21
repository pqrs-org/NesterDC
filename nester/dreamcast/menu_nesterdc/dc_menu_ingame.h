#ifndef _DC_MENU_INGAME_H_
#define _DC_MENU_INGAME_H_

typedef enum {
  DC_MENU_INGAME_STATESAVE_QUICK,
  DC_MENU_INGAME_STATESAVE_VMU,
} dc_menu_ingame_statesave_type_t;

void dc_menu_ingame_cheat (NES *emu);
void dc_menu_ingame_option (NES *emu);
void dc_menu_ingame_statesave (NES *emu, dc_menu_ingame_statesave_type_t ss_type, maple_device_t *dev);
void dc_menu_ingame_system (NES *emu);


#endif

