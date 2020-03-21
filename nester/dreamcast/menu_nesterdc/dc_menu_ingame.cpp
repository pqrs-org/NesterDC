#include "nes.h"
#include "dc_utils.h"
#include "dc_main.h"
#include "dc_menu_common.h"
#include "dc_menu_screenadjust.h"
#include "dc_menu_filelist_cdfs.h"
#include "dc_menu_filelist_nnnesterj_cheatcode.h"
#include "dc_gg.h"
#include "dc_saving.h"
#include "dc_menu_ingame.h"


/* ============================================================ */
typedef void (*dc_menu_ingame_fgfunc)(NES *emu);

typedef struct {
  const char *title;
  const char *head;
  const char *str_Y;
  const char *str_X;
  const char *str_B;
  const char *str_A;
  dc_menu_ingame_fgfunc fgfunc;
} dc_menu_ingame_display_info_t;


/* ============================================================ */
static int
dc_menu_ingame_calc_center (const char *str)
{
  int len = strlen(str);
  const int font_width = 12;
  
  return (640 - len * font_width) / 2;
}


static void
dc_menu_ingame_display_common (dc_menu_ingame_display_info_t *info, NES *emu)
{
  int x;
  int y;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 Y_color = 0xff00ff00;
  uint32 X_color = 0xffffff00;
  uint32 B_color = 0xff0000ff;
  uint32 A_color = 0xffff0000;
  uint32 w_color = 0xd0010101;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  dc_menu_ingame_bgfunc();
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  
  if (info->title)
  {
    dc_pvr_font_commit_string (20, 50, 512, f_color, info->title);
    dc_pvr_font_commit_string (21, 51, 510, b_color, info->title);
  }
  
  if (info->head)
    dc_pvr_font_commit_string (20, 100, 512, f_color, info->head);
  
  x = dc_menu_ingame_calc_center (info->str_Y);
  y = 150;
  dc_pvr_font_commit_string (x, y, 500, f_color, info->str_Y);
  dc_pvr_font_commit_string (x + 1, y + 1, 400, Y_color, info->str_Y);
  
  x = 50;
  y += 24;
  y += 24;
  dc_pvr_font_commit_string (x, y, 500, f_color, info->str_X);
  dc_pvr_font_commit_string (x + 1, y + 1, 400, X_color, info->str_X);
  x = 450;
  dc_pvr_font_commit_string (x, y, 500, f_color, info->str_B);
  dc_pvr_font_commit_string (x + 1, y + 1, 400, B_color, info->str_B);
  
  x = dc_menu_ingame_calc_center (info->str_A);
  y += 24;
  y += 24;
  dc_pvr_font_commit_string (x, y, 500, f_color, info->str_A);
  dc_pvr_font_commit_string (x + 1, y + 1, 400, A_color, info->str_A);
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  dc_pvr_diagram_commit_box (0, 0, 640, 480, 300, w_color);
  
  if (info->fgfunc) info->fgfunc(emu);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


/* ============================================================ */
void
dc_menu_ingame_cheat (NES *emu)
{
  dc_menu_controller_status_t *p;
  dc_menu_ingame_display_info_t info;
  
  info.title = "Cheat Menu";
  info.head = "Game Genie / NNNesterJ Cheat Code";
  info.str_Y = "Y: Set NNNesterJ Cheat Checkflag";
  info.str_X = "X: Load NNNesterJ Cheat";
  info.str_B = "B: Exit";
  info.str_A = "A: Load Genie";
  info.fgfunc = NULL;
  
  emu->freeze();
  dc_menu_controller_status_init ();
  dc_menu_ingame_init ();
  
  for (;;) 
  {
    dc_menu_controller_status_scan ();
    dc_menu_ingame_display_common (&info, emu);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypress(p->B))
      {
	timer_spin_sleep (100);
	goto finish;
      }
      
      if (dc_menu_keypress(p->A))
      {
	char result[256];
	
	dc_menu_filelist_cdfs (result, sizeof(result), "/cd/genie", dc_menu_ingame_bgfunc);
	
	if (result[0])
        {
	  load_genie_from_file (result, emu);
          timer_spin_sleep (100);
          goto finish;
        }
      }
      
      if (dc_menu_keypress(p->X))
      {
        char result[256];
        
        dc_menu_filelist_cdfs (result, sizeof(result), "/cd/nnnesterj_cheat", dc_menu_ingame_bgfunc);
        
        if (result[0]) 
        {
          emu->load_nnnesterj_cheat (result);
          dc_menu_controller_status_init ();
        }
      }
      
      if (dc_menu_keypress(p->Y))
      {
        dc_menu_filelist_nnnesterj_cheatcode (emu->nes_nnnesterj_cheatinfo, 
                                              emu->nes_nnnesterj_cheatinfo_size,
                                              dc_menu_ingame_bgfunc);
        emu->set_nnnesterj_cheat_alive_num ();
        
        dc_menu_controller_status_init ();
      }
      
      ++p;
    }
  }
  
finish:  
  emu->thaw();
}


/* ============================================================ */
static void
dc_menu_ingame_option_fgfunc (NES *emu)
{
  uint32 f_color = 0xffffffff;
  char str[256];
  int y;
  
  dc_pvr_font_commit_start ();
  
  y = 300;
  sprintf (str, "START: Bilinear Filter [%s]", 
	   dc_settings.enable_bilinear_filter ? "Enable" : "Disable");
  dc_pvr_font_commit_string (50, y, 500, f_color, str);
  
  sprintf (str, "L: Autofire-mode [%s]", 
	   dc_settings.enable_autofire ? "Enable" : "Disable");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, str);
  
  sprintf (str, "Autofire B [%s]", 
	   dc_settings.enable_autofire_b ? "Enable" : "Disable");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, str);
  
  sprintf (str, "Autofire A [%s]", 
	   dc_settings.enable_autofire_a ? "Enable" : "Disable");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, str);
}


void
dc_menu_ingame_option (NES *emu)
{
  dc_menu_controller_status_t *p;
  dc_menu_ingame_display_info_t info;
  
  info.title = "Option Menu";
  info.head = NULL;
  info.str_Y = "Y: Adjust Screen Position";
  info.str_X = "X: Toggle Autofire B";
  info.str_B = "B: Exit";
  info.str_A = "A: Toggle Autofire A";
  info.fgfunc = dc_menu_ingame_option_fgfunc;
  
  emu->freeze();
  dc_menu_controller_status_init ();
  dc_menu_ingame_init ();
  
  for (;;) 
  {
    dc_menu_controller_status_scan ();
    dc_menu_ingame_display_common (&info, emu);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypress(p->B))
      {
	timer_spin_sleep (100);
	goto finish;
      }
      
      if (dc_menu_keypress(p->Y))
      {
	dc_menu_screenadjust (dc_menu_ingame_bgfunc);
        dc_menu_controller_status_init ();
      }
      
      if (dc_menu_keypress(p->A))
	dc_settings.enable_autofire_a = !(dc_settings.enable_autofire_a);
      
      if (dc_menu_keypress(p->X))
	dc_settings.enable_autofire_b = !(dc_settings.enable_autofire_b);
      
      if (dc_menu_keypress(p->START))
	dc_settings.enable_bilinear_filter = !(dc_settings.enable_bilinear_filter);
      
      if (dc_menu_keypress(p->ltrig))
	dc_settings.enable_autofire = !(dc_settings.enable_autofire);
      
      ++p;
    }
  }
  
finish:
  emu->thaw();
}


/* ============================================================ */
void
dc_menu_ingame_statesave (NES *emu, dc_menu_ingame_statesave_type_t ss_type, maple_device_t *dev)
{
  dc_menu_controller_status_t *p;
  dc_menu_ingame_display_info_t info;
  const char *str;
  
  if (ss_type == DC_MENU_INGAME_STATESAVE_VMU)
  {
    info.title = "VMU StateSave Menu";
    info.head = "You can save StateSave to VMU";
  }
  else
  {
    info.title = "Quick StateSave Menu";
    info.head = "You can save StateSave to memory";
  }
  info.str_Y = "Y: ";
  info.str_X = "X: Save";
  info.str_B = "B: Exit";
  info.str_A = "A: Load";
  info.fgfunc = NULL;
  
  emu->freeze();
  dc_menu_controller_status_init ();
  dc_menu_ingame_init ();
  
  for (;;) 
  {
    dc_menu_controller_status_scan ();
    dc_menu_ingame_display_common (&info, emu);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypress(p->B))
      {
	timer_spin_sleep (100);
	goto finish;
      }
      
      if (dc_menu_keypress(p->X))
      {
        bool save_success;
        
	dc_pvr_font_output_message (info.title, "Now Saving...", NULL, dc_menu_ingame_bgfunc); 
	
        if (ss_type == DC_MENU_INGAME_STATESAVE_VMU)
          save_success = snap_save_to_vmu(dev, emu);
        else
          save_success = emu->saveState("/md/quick");
        
        if (save_success)
          str = "Save Done";
        else
          str = "Save Failed";
        dc_pvr_font_output_message (info.title, str, NULL, dc_menu_ingame_bgfunc);
	
	timer_spin_sleep (1000);
	goto finish;
      }
      
      if (dc_menu_keypress(p->A))
      {
        bool load_success;
        
	dc_pvr_font_output_message (info.title, "Now Loading...", NULL, dc_menu_ingame_bgfunc); 
	
        if (ss_type == DC_MENU_INGAME_STATESAVE_VMU)
          load_success = snap_load_from_vmu (dev, emu);
        else
          load_success = emu->loadState("/md/quick");
        
	if (load_success)
          str = "Load Done";
	else
          str = "Load Failed";
        dc_pvr_font_output_message (info.title, str, NULL, dc_menu_ingame_bgfunc); 
	
	timer_spin_sleep (1000);
	goto finish;
      }
      
      ++p;
    }
  }
  
finish:
  emu->thaw();
}


/* ============================================================ */
static void
dc_menu_ingame_system_fgfunc (NES *emu)
{
  uint32 f_color = 0xffffffff;
  char str[256];
  int y;
  
  dc_pvr_font_commit_start ();
  
  y = 300;
  sprintf(str, "Current External Device: %s", emu->get_external_device_name());
  dc_pvr_font_commit_string (50, y, 500, f_color, str);
  
  if (emu->get_rom_image_type () == NES_ROM::fds_image)
  {
    const char *disksidestr[] = {0, "1A", "1B", "2A", "2B"};
    
    sprintf(str, "Currnet DiskSide: %s", disksidestr[emu->GetDiskSide ()]);
    
    y += 24;
    y += 24;
    dc_pvr_font_commit_string (50, y, 500, f_color, "UP/DOWN/LEFT/RIGHT: Set DiskSide 1A/1B/2A/2B");
    y += 24;
    dc_pvr_font_commit_string (50, y, 500, f_color, str);
  }
}


void
dc_menu_ingame_system (NES *emu)
{
  dc_menu_controller_status_t *p;
  dc_menu_ingame_display_info_t info;
  
  info.title = "System Menu";
  info.head = NULL;
  info.str_Y = "Y: Reset";
  info.str_X = "X: Change External Device";
  info.str_B = "B: Exit";
  info.str_A = "A: ";
  info.fgfunc = dc_menu_ingame_system_fgfunc;
  
  emu->freeze();
  dc_menu_controller_status_init ();
  dc_menu_ingame_init();
  
  for (;;) 
  {
    dc_menu_controller_status_scan ();
    dc_menu_ingame_display_common (&info, emu);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypress(p->B))
      {
	timer_spin_sleep (100);
	goto finish;
      }
      
      if (dc_menu_keypress(p->X))
	emu->change_external_device ();
      
      if (dc_menu_keypress(p->Y))
      {
	emu->reset();
	timer_spin_sleep (100);
	goto finish;
      }
      
      if (emu->get_rom_image_type () == NES_ROM::fds_image)
      {
        if (dc_menu_keypress(p->DPAD_UP))
	  emu->SetDiskSide (0x01);
        if (dc_menu_keypress(p->DPAD_DOWN))
	  emu->SetDiskSide (0x02);
        if (dc_menu_keypress(p->DPAD_LEFT))
	  emu->SetDiskSide (0x03);
        if (dc_menu_keypress(p->DPAD_RIGHT))
	  emu->SetDiskSide (0x04);
      }
      
      ++p;
    }
  }
  
finish:  
  emu->thaw();
}



