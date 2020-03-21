#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_filelist.h"

/* ============================================================ */
/* global settings */
dc_menu_filelist_global_config_t dc_menu_filelist_global_config;

void
dc_menu_filelist_global_config_init ()
{
  dc_menu_filelist_global_config.cursor_wait = 10;
  dc_menu_filelist_global_config.pageup_wait = 10;
}


/* ------------------------------------------------------------ */
dc_menu_filelist_global_dirstatus_t dc_menu_filelist_global_dirstatus[DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE];

static void
dc_menu_filelist_global_dirstatus_init ()
{
  int i;
  dc_menu_filelist_global_dirstatus_t *p;
  
  p = dc_menu_filelist_global_dirstatus;
  for (i = 0; i < DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE; ++i)
  {
    memset(p->path, 0, sizeof(p->path));
    p->num_files = 0;
    p->base_pos = 0;
    p->cur_pos = 0;
    
    ++p;
  }
}


/* ------------------------------------------------------------ */
void
dc_menu_filelist_init ()
{
  dc_menu_filelist_global_config_init ();
  dc_menu_filelist_global_dirstatus_init ();
}


/* ============================================================ */
static dc_menu_filelist_global_dirstatus_t *
find_dirstatus (const char *path)
{
  int i;
  dc_menu_filelist_global_dirstatus_t *p;
  
  p = dc_menu_filelist_global_dirstatus;
  for (i = 0; i < DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE; ++i)
  {
    if (!strcmp(path, p->path)) return p;
    ++p;
  }
  
  return NULL;
}


static dc_menu_filelist_global_dirstatus_t *
find_free_filelist_dirstatus ()
{
  static int dirstatus_index = 0;
  dc_menu_filelist_global_dirstatus_t *rv;
  
  rv = dc_menu_filelist_global_dirstatus + dirstatus_index;
  dirstatus_index = (dirstatus_index + 1) & (DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE - 1);
  
  rv->num_files = 0;
  rv->base_pos = 0;
  rv->cur_pos = 0;
  
  return rv;
}


void
dc_menu_filelist_info_init(dc_menu_filelist_info_t *info, dc_menu_filelist_iteminfo_t *iteminfo, const char *path)
{
  dc_menu_filelist_iteminfo_t *p;
  int iteminfo_size;
  
  info->title = NULL;
  info->foot = NULL;
  info->bgfunc = NULL;
  
  info->selectable = 0;
  info->is_display_filesize = 0;
  info->keyfunc = NULL;
  
  /* init iteminfo */
  info->iteminfo = iteminfo;
  
  iteminfo_size = 0;
  p = iteminfo;
  while (p->type != FILETYPE_NULL) 
  {
    ++iteminfo_size;
    ++p;
  }
  
  /* init dirstatus */  
  info->dirstatus = find_dirstatus (path);
  if (info->dirstatus == NULL)
  {
    info->dirstatus = find_free_filelist_dirstatus ();
    sprintf(info->dirstatus->path, "%s", path);
  }
  info->dirstatus->num_files = iteminfo_size;
  if (info->dirstatus->base_pos > iteminfo_size) info->dirstatus->base_pos = 0;
  if (info->dirstatus->cur_pos > iteminfo_size) info->dirstatus->cur_pos = 0;
}



static void
display_menu (dc_menu_filelist_info_t *info)
{
  int n;
  int y;
  char str[256];
  dc_menu_filelist_iteminfo_t *p;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 s_color = 0xff00ffff;
  uint32 c_color = 0xff0000ff;
  uint32 w_color = 0xb0010101;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (info->bgfunc) (info->bgfunc)();
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  
  if (info->title)
  {
    dc_pvr_font_commit_string (20, 50, 512, f_color, info->title);
    dc_pvr_font_commit_string (21, 51, 510, b_color, info->title);
  }
  
  dc_pvr_font_commit_string (20, 100, 512, f_color, info->dirstatus->path);
  
  y = 150;
  p = info->iteminfo + info->dirstatus->base_pos;
  for (n = 0; n < DRAW_LINES; ++n)
  {
    uint32 color;
    
    if (p->type == FILETYPE_NULL) break;
    
    if (p->selected)
      color = s_color;
    else
      color = f_color;
    
    dc_pvr_font_commit_string (50, y, 512, color, p->text);
    if (info->is_display_filesize)
    {
      sprintf(str, "(%d)", p->filesize);
      dc_pvr_font_commit_string (560, y, 512, color, str);
    }
    
    y += 24;
    ++p;
  }
  
  if (info->foot)
  {
    dc_pvr_font_commit_string (20, 400, 512, f_color, info->foot);
    dc_pvr_font_commit_string (20, 401, 510, b_color, info->foot);
  }
  
  sprintf (str, "Cursor Wait: %3d, Pageup Wait: %3d", 
           dc_menu_filelist_global_config.cursor_wait, 
           dc_menu_filelist_global_config.pageup_wait);
  dc_pvr_font_commit_string (20, 440, 512, f_color, str);
  dc_pvr_font_commit_string (20, 441, 510, b_color, str);
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  dc_pvr_diagram_commit_box (0, 0, 640, 480, 300, w_color);
  
  /* draw cursor */
  y = 150 + (info->dirstatus->cur_pos - info->dirstatus->base_pos) * 24 + 12;
  dc_pvr_diagram_commit_line (50, y, 600, y, 500, c_color);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


void
dc_menu_filelist_display_nowloading(dc_pvr_bgfunc_t bgfunc)
{
  int i;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 w_color = 0xb0010101;
  
  /* fill PVR cache */
  for (i = 0; i < 2; ++i) 
  {
    pvr_wait_ready ();
    pvr_scene_begin ();
    
    pvr_list_begin (PVR_LIST_OP_POLY);
    if (bgfunc) (bgfunc)();
    pvr_list_finish ();
    
    pvr_list_begin (PVR_LIST_TR_POLY);
    dc_pvr_font_commit_start ();
    
    dc_pvr_font_commit_string (20, 100, 512, f_color, "Now Loading");
    
    dc_pvr_diagram_commit_start ();
    dc_pvr_diagram_commit_line (20, 80, 600, 80, 500, b_color);
    dc_pvr_diagram_commit_line (20, 430, 600, 430, 500, b_color);
    
    dc_pvr_diagram_commit_box (0, 0, 640, 480, 300, w_color);
    pvr_list_finish ();
    
    pvr_scene_finish ();
  }
}


/* ------------------------------------------------------------ */
dc_menu_filelist_result_type_t
dc_menu_filelist_common (dc_menu_filelist_info_t *info)
{
  dc_menu_controller_status_t *p;
  dc_menu_filelist_global_dirstatus_t *ds = info->dirstatus;
  int in_select_mode = 0;
  int selected = 0;
  
  dc_menu_controller_status_init ();
  
  for (;;)
  {
    dc_menu_controller_status_scan ();
    display_menu (info);
    
    /* set select mode (if any L-trigger pulled, then in select-mode) */
    p = dc_menu_controller_status;
    in_select_mode = 0;
    if (info->selectable)
    {
      while (p->dev)
      {
        in_select_mode |= dc_menu_keypressing(p->ltrig);
        
        if (dc_menu_keypress(p->ltrig))
        {
          info->iteminfo[ds->cur_pos].selected = !(info->iteminfo[ds->cur_pos].selected);
          selected = info->iteminfo[ds->cur_pos].selected;
        }
        
        ++p;
      }
    }
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      /* set nowait-mode */
      if (dc_menu_keypressing(p->X))
      {
#define DMF_RELEASE_KEY(BUTTON) { \
        if (p->DPAD_##BUTTON >= DC_MENU_CONTROLLER_STATUS_PRESS) \
          p->DPAD_##BUTTON = DC_MENU_CONTROLLER_STATUS_PRESS; \
        }
        
        DMF_RELEASE_KEY(UP);
        DMF_RELEASE_KEY(DOWN);
        DMF_RELEASE_KEY(LEFT);
        DMF_RELEASE_KEY(RIGHT);
      }
      
      /* set cursor-wait */
      if (p->DPAD_UP == dc_menu_filelist_global_config.cursor_wait)
        p->DPAD_UP = DC_MENU_CONTROLLER_STATUS_PRESS;
      if (p->DPAD_DOWN == dc_menu_filelist_global_config.cursor_wait)
        p->DPAD_DOWN = DC_MENU_CONTROLLER_STATUS_PRESS;
      if (p->DPAD_LEFT == dc_menu_filelist_global_config.pageup_wait)
        p->DPAD_LEFT = DC_MENU_CONTROLLER_STATUS_PRESS;
      if (p->DPAD_RIGHT == dc_menu_filelist_global_config.pageup_wait)
        p->DPAD_RIGHT = DC_MENU_CONTROLLER_STATUS_PRESS;
      
      /* ---------------------------------------- */
      if (dc_menu_keypressing(p->rtrig) && dc_menu_keypressing(p->ltrig))
        return FILELIST_ABORT;
      
      if (dc_menu_keypress(p->DPAD_UP) || dc_menu_keypress(p->DPAD_LEFT))
      {
        int i, count;
        
        if (dc_menu_keypress(p->DPAD_UP))
          count = 1;
        else
          count = DRAW_LINES;
        
        for (i = 0; i < count; ++i)
        {
          if (ds->cur_pos > 0)
          {
            if (ds->cur_pos == ds->base_pos) --(ds->base_pos);
            --(ds->cur_pos);
            
            if (in_select_mode) 
              info->iteminfo[ds->cur_pos].selected = selected;
          }
        }
      }
      
      if (dc_menu_keypress(p->DPAD_DOWN) || dc_menu_keypress(p->DPAD_RIGHT))
      {
        int i, count;
        
        if (dc_menu_keypress(p->DPAD_DOWN))
          count = 1;
        else
          count = DRAW_LINES;
        
        for (i = 0; i < count; ++i)
        {
          if (info->iteminfo[ds->cur_pos + 1].type != FILETYPE_NULL)
          {
            if (ds->cur_pos == ds->base_pos + DRAW_LINES - 1) ++(ds->base_pos);
            ++(ds->cur_pos);
            
            if (in_select_mode)
              info->iteminfo[ds->cur_pos].selected = selected;
          }
        }
      }
      
      ++p;
    }
    
    if (info->keyfunc) 
    {
      dc_menu_filelist_result_type_t result_type;
      
      result_type = (info->keyfunc)(info->dirstatus);
      
      if (result_type != FILELIST_NOTHING)
        return result_type;
    }
  }
}


