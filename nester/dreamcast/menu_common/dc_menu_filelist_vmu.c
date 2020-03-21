#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_filelist.h"
#include "dc_menu_filelist_vmu.h"

#include "dc_vmu.h"

#define DC_MENU_FILELIST_VMU_ITEMINFO_SIZE 128
static dc_menu_filelist_iteminfo_t iteminfo[DC_MENU_FILELIST_VMU_ITEMINFO_SIZE];

/* ============================================================ */
static void
display_help (dc_pvr_bgfunc_t bgfunc)
{
  int y;
  uint32 f_color = 0xffffffff;
  uint32 b_color = 0xffffa500;
  uint32 w_color = 0xb0010101;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  pvr_list_begin (PVR_LIST_OP_POLY);
  if (bgfunc) bgfunc();
  pvr_list_finish ();
  
  pvr_list_begin (PVR_LIST_TR_POLY);
  dc_pvr_font_commit_start ();
  dc_pvr_font_commit_string (20, 50, 512, f_color, "Help");
  dc_pvr_font_commit_string (21, 51, 510, b_color, "Help");
  
  y = 150;
  dc_pvr_font_commit_string (50, y, 512, f_color, "L: Select File");
  y += 24;
  dc_pvr_font_commit_string (50, y, 512, f_color, "R + Y: Remove Selected Files");
  y += 24;
  dc_pvr_font_commit_string (50, y, 512, f_color, "L + R: Exit VMU Menu");
  y += 24;
  dc_pvr_font_commit_string (50, y, 512, f_color, "START: Exit Help");
  
  dc_pvr_diagram_commit_start ();
  y = 80;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  y = 430;
  dc_pvr_diagram_commit_line (20, y, 600, y, 500, b_color);
  
  dc_pvr_diagram_commit_box (20, 80, 600, 430, 400, w_color);
  
  pvr_list_finish ();
  pvr_scene_finish ();
}


static void
help(dc_pvr_bgfunc_t bgfunc)
{
  dc_menu_controller_status_t *p;
  
  dc_menu_controller_status_init ();
  
  for (;;)
  {
    dc_menu_controller_status_scan ();
    display_help (bgfunc);
    
    p = dc_menu_controller_status;
    while (p->dev)
    {
      if (dc_menu_keypress(p->START)) return;
      ++p;
    }
  }
}


/* ============================================================ */
static void
read_vmu (char *title, int title_len, maple_device_t *dev)
{
  int num_entries;
  dirent_vmu entries[DC_MENU_FILELIST_VMU_ITEMINFO_SIZE];
  int i;
  dc_menu_filelist_iteminfo_t *p;
  
  memset(iteminfo, 0, sizeof(iteminfo));
  
  num_entries = DC_MENU_FILELIST_VMU_ITEMINFO_SIZE;
  if (ndc_vmu_getall_dirent (entries, &num_entries, dev) < 0) return;
  
  p = iteminfo;
  for (i = 0; i < num_entries; ++i)
  {
    if (entries[i].filetype != 0x00)
    {
      uint8 buf[512];
      file_hdr_vmu *hdr = (file_hdr_vmu *)buf;
      
      if (vmu_block_read (dev, entries[i].firstblk, buf) < 0)
        continue;
      
      if (strcmp(hdr->app_id, progname))
        continue;
      
      /* filename and desc_long is not NULL terminated. */
      strncpy (p->path, entries[i].filename, 12);
      p->path[12] = '\0';
      
      strncpy (p->text, hdr->desc_long, 32);
      p->text[32] = '\0';
      
      p->type = FILETYPE_FILE;
      p->selected = 0;
      p->filesize = entries[i].filesize;
      
      ++p;
    }
  }
  p->type = FILETYPE_NULL;
  
  sprintf (title, "VMU Menu (%d Blocks Free)", 
           ndc_vmu_check_free_blocks (NULL, dev));
}


static dc_menu_filelist_result_type_t
dc_menu_filelist_vmu_keyfunc(dc_menu_filelist_global_dirstatus_t *dirstatus)
{
  dc_menu_controller_status_t *p;
  
  p = dc_menu_controller_status;
  while (p->dev)
  {
    if (dc_menu_keypress(p->START))
      help(NULL);
    
    if (dc_menu_keypressing(p->Y) && dc_menu_keypressing(p->rtrig))
      return FILELIST_FINISH;
    
    ++p;
  }
  
  return FILELIST_NOTHING;
}


void
dc_menu_filelist_vmu (maple_device_t *dev, dc_pvr_bgfunc_t bgfunc)
{
  dc_menu_filelist_info_t info;
  dc_menu_filelist_result_type_t result_type;
  char title[256];
  
  dc_menu_filelist_display_nowloading (bgfunc);
  
  read_vmu (title, sizeof(title), dev);
  
  dc_menu_filelist_info_init (&info, iteminfo, "/vmu");
  
  info.title = title;
  info.foot = "Press START to HELP";
  info.bgfunc = bgfunc;
  info.selectable = 1;
  info.is_display_filesize = 1;
  info.keyfunc = dc_menu_filelist_vmu_keyfunc;
  
  result_type = dc_menu_filelist_common (&info);
  
  if (result_type == FILELIST_FINISH)
  {
    dc_menu_filelist_iteminfo_t *p;
    char str[256];
    
    p = iteminfo;
    while (p->type != FILETYPE_NULL)
    {
      if (p->selected)
      {
        sprintf (str, "Removing %s", p->text);
        dc_pvr_font_output_message ("VMU Menu", str, NULL, bgfunc);
        ndc_vmu_remove_file (dev, p->path);
      }
      
      ++p;
    }
  }
}


