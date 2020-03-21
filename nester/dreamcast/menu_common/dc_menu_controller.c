#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_controller.h"


static int dc_maple_controller_virtual_index[DC_MAPLE_INFO_SIZE];
static int dc_maple_mouse_virtual_index[DC_MAPLE_INFO_SIZE];
static int dc_maple_keyboard_virtual_index[DC_MAPLE_INFO_SIZE];

static int dc_maple_controller_info_size;
static int dc_maple_mouse_info_size;
static int dc_maple_keyboard_info_size;


void
dc_menu_controller_init()
{
  int i; 
  
  for (i = 0; i < DC_MAPLE_INFO_SIZE; ++i)
  {
    dc_maple_controller_virtual_index[i] = i;
    dc_maple_mouse_virtual_index[i] = i;
    dc_maple_keyboard_virtual_index[i] = i;
  }
}


static int
calc_dc_maple_info_size(dc_maple_info_t *p)
{
  int size;
  
  size = 0;
  while (p->dev) 
  {
    ++size;
    ++p;
  }
  
  return size;
}


static void
set_virtual_index (int next, int physical_index)
{
  int i;
  int old = dc_maple_controller_virtual_index[physical_index];
  
  for (i = 0; i < DC_MAPLE_INFO_SIZE; ++i)
  {
    if (dc_maple_controller_virtual_index[i] == next)
    {
      dc_maple_controller_virtual_index[i] = old;
      break;
    }
  }
  
  dc_maple_controller_virtual_index[physical_index] = next;
}


static void
dc_menu_maple_scan ()
{
  int old_size, new_size;
  
  dc_maple_init ();
  
  old_size = dc_maple_controller_info_size;
  new_size = calc_dc_maple_info_size (dc_maple_controller_info);
  if (old_size > new_size) dc_menu_controller_init ();
  dc_maple_controller_info_size = new_size;
  
  old_size = dc_maple_mouse_info_size;
  new_size = calc_dc_maple_info_size (dc_maple_mouse_info);
  if (old_size > new_size) dc_menu_controller_init ();
  dc_maple_mouse_info_size = new_size;
  
  old_size = dc_maple_keyboard_info_size;
  new_size = calc_dc_maple_info_size (dc_maple_keyboard_info);
  if (old_size > new_size) dc_menu_controller_init ();
  dc_maple_keyboard_info_size = new_size;
}


static void
dc_menu_sync_to_virtual_port ()
{
  dc_maple_info_t dmi[DC_MAPLE_INFO_SIZE];
  dc_maple_info_t *p;
  int *vp;
  int i;
  
  p = dc_maple_controller_info;
  vp = dc_maple_controller_virtual_index;
  
  /* backup all controller info */
  for (i = 0; i < DC_MAPLE_INFO_SIZE; ++i)
    dmi[i] = p[i];
  
  for (i = 0; i < DC_MAPLE_INFO_SIZE; ++i)
    p[vp[i]] = dmi[i];
}


static void
display_menu ()
{
  int n;
  int y; 
  dc_maple_info_t *p;
  int *vp;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  char str[256];
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  pvr_list_finish ();
  
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  dc_pvr_font_commit_string (20, 50, 512, f_color, "Detected Controllers");
  dc_pvr_font_commit_string (21, 51, 510, b_color, "Detected Controllers");
  
  n = 0;
  y = 150;
  p = dc_maple_controller_info;
  vp = dc_maple_controller_virtual_index;
  while (p->dev)
  {
    if (p->type.c_type == DC_MAPLE_CONTROLLER_ARCADESTICK)
      sprintf(str, "%d: Arcade Stick", n);
    else
      sprintf(str, "%d: Dreamcast Controller", n);
    dc_pvr_font_commit_string (50, y, 512, f_color, str);
    sprintf (str, "as Pad #%d", vp[n]);
    dc_pvr_font_commit_string (400, y, 512, f_color, str);
    
    ++n;
    ++p;
    y += 24;
  }
  
  n = 0;
  p = dc_maple_mouse_info;
  vp = dc_maple_mouse_virtual_index;
  while (p->dev)
  {
    sprintf (str, "%d: Dreamcast Mouse", n);
    dc_pvr_font_commit_string (50, y, 512, f_color, str);
    sprintf(str, "as Mouse #%d", vp[n]);
    dc_pvr_font_commit_string (400, y, 512, f_color, str);
    
    ++n;
    ++p;
    y += 24;
  }
  
  n = 0;
  p = dc_maple_keyboard_info;
  vp = dc_maple_keyboard_virtual_index;
  while (p->dev)
  {
    sprintf(str, "%d: Dreamcast Keyboard", n);
    dc_pvr_font_commit_string (50, y, 512, f_color, str);
    sprintf(str, "as Keyboard #%d", vp[n]);
    dc_pvr_font_commit_string (400, y, 512, f_color, str);
    
    ++n;
    ++p;
    y += 24;
  }
  
  y += 24;
  dc_pvr_font_commit_string (30, y, 512, f_color, "Plug controllers now");
  y += 24;
  y += 24;
  dc_pvr_font_commit_string (30, y, 512, f_color, "Press X/Y to change virtual index");
  y += 24;
  dc_pvr_font_commit_string (30, y, 512, f_color, "Press B to reset virtual index");
  
  dc_pvr_font_commit_string (400, 400, 512, f_color, "Press START");
  dc_pvr_font_commit_string (401, 401, 510, b_color, "Press START");
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


void
dc_menu_controller()
{
  int physical_index;
  dc_menu_controller_status_t *p;
  
  dc_menu_maple_scan ();
  dc_menu_controller_status_init ();
  
  for (;;)
  {
    dc_menu_maple_scan ();
    dc_menu_controller_status_scan ();
    display_menu();
    
    p = dc_menu_controller_status;
    physical_index = 0;
    while (p->dev)
    {
      if (dc_menu_keypress(p->B))
        set_virtual_index (physical_index, physical_index);
      
      if (dc_menu_keypress(p->X))
      {
        int next = dc_maple_controller_virtual_index[physical_index] + 1;
        if (next == dc_maple_controller_info_size) next = 0;
        set_virtual_index (next, physical_index);
      }
      
      if (dc_menu_keypress(p->Y))
      {
        int next = dc_maple_controller_virtual_index[physical_index] - 1;
        if (next < 0) next = dc_maple_controller_info_size - 1;
        set_virtual_index (next, physical_index);
      }
      
      if (dc_menu_keypress(p->START))
        goto finish;
      
      ++p;
      ++physical_index;
    }
  }
  
finish:
  dc_menu_sync_to_virtual_port ();
}


