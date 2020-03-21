#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_screenadjust.h"


static void
display_menu (dc_pvr_bgfunc_t bgfunc)
{
  int y;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 c_color = 0xff0000ff;
  uint32 w_color = 0xd0010101;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (bgfunc) bgfunc();
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  
  dc_pvr_font_commit_string (20, 50, 512, f_color, "Screen Adjust");
  dc_pvr_font_commit_string (21, 51, 510, b_color, "Screen Adjust");
  
  y = 150;
  dc_pvr_font_commit_string (50, y, 500, f_color, "Up/Down/Left/Right: Adjust Position");
  y += 24; 
  dc_pvr_font_commit_string (50, y, 500, f_color, "X: Speedup");
  y += 24; 
  dc_pvr_font_commit_string (50, y, 500, f_color, "R + Y: Reset Settings");
  y += 24; 
  dc_pvr_font_commit_string (50, y, 500, f_color, "R + B: Set Default");
  y += 24;
  dc_pvr_font_commit_string (50, y, 500, f_color, "L + R: Exit");
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  dc_pvr_diagram_commit_box (0, 0, 640, 480, 300, w_color);
  
  /* draw cross */
  dc_pvr_diagram_commit_line (0, 0, 640, 480, 500, c_color);
  dc_pvr_diagram_commit_line (0, 480, 640, 0, 500, c_color);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


void
dc_menu_screenadjust (dc_pvr_bgfunc_t bgfunc)
{
  dc_screen_offset_t dso_orig;
  dc_menu_controller_status_t *p;
  int adjust_size;
  
  dso_orig = dc_screen_offset;
  dc_menu_controller_status_init();
  
  for (;;)
  {
    dc_menu_controller_status_scan ();
    display_menu (bgfunc);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->ltrig))
        return;
      
      if (dc_menu_keypressing(p->X))
        adjust_size = 2;
      else
        adjust_size = 1;
      
      if (dc_menu_keypressing(p->DPAD_UP))
        dc_screen_offset.y -= adjust_size;
      
      if (dc_menu_keypressing(p->DPAD_DOWN))
        dc_screen_offset.y += adjust_size;
      
      if (dc_menu_keypressing(p->DPAD_LEFT))
        dc_screen_offset.x -= adjust_size;
      
      if (dc_menu_keypressing(p->DPAD_RIGHT))
        dc_screen_offset.x += adjust_size;
      
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->Y))
        dc_screen_offset = dso_orig;
      
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->B))
        dc_screen_offset_init ();
      
      ++p;
    }
  }  
}


