#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_filelist.h"
#include "dc_menu_filelist_vmuselect.h"

#include "dc_vmu.h"

#define DC_MENU_FILELIST_VMUSELECT_ITEMINFO_SIZE 32
static dc_menu_filelist_iteminfo_t iteminfo[DC_MENU_FILELIST_VMUSELECT_ITEMINFO_SIZE];
static maple_device_t *iteminfo_dev[DC_MENU_FILELIST_VMUSELECT_ITEMINFO_SIZE];


/* ============================================================ */
static void
read_vmu_addrs ()
{
  int n;
  dc_menu_filelist_iteminfo_t *p;
  maple_device_t *dev;
  maple_device_t **q;
  
  memset (iteminfo, 0, sizeof(iteminfo));
  
  n = 0;
  p = iteminfo;
  q = iteminfo_dev;
  for (;;)
  {
    dev = maple_enum_type(n, MAPLE_FUNC_MEMCARD);
    if (!dev) break;
    
    p->path[0] = '\0';
    sprintf (p->text, "VMU%d: %3d Blocks Free", n, ndc_vmu_check_free_blocks (NULL, dev));
    p->type = FILETYPE_FILE;
    p->selected = 0;
    p->filesize = 0;
    *q = dev;
    
    ++n;
    ++p;
    ++q;
  }
  p->type = FILETYPE_NULL;
}


static dc_menu_filelist_result_type_t
dc_menu_filelist_vmuselect_keyfunc(dc_menu_filelist_global_dirstatus_t *dirstatus)
{
  dc_menu_controller_status_t *p;
  
  p = dc_menu_controller_status;
  while (p->dev)
  {
    if (dc_menu_keypress(p->START))
      return FILELIST_FINISH;
    
    ++p;
  }
  
  return FILELIST_NOTHING;
}


maple_device_t *
dc_menu_filelist_vmuselect ()
{
  dc_menu_filelist_info_t info;
  dc_menu_filelist_result_type_t result_type;
  
  dc_menu_filelist_display_nowloading (NULL);
  
  read_vmu_addrs ();
  
  dc_menu_filelist_info_init (&info, iteminfo, "/vmuselect");
  
  info.title = "VMU Select";
  info.foot = "Press START";
  info.bgfunc = NULL;
  info.selectable = 0;
  info.is_display_filesize = 0;
  info.keyfunc = dc_menu_filelist_vmuselect_keyfunc;
  
  result_type = dc_menu_filelist_common (&info);
  
  if (result_type == FILELIST_FINISH)
    return iteminfo_dev[(info.dirstatus)->cur_pos];
  else
    return NULL;
}


