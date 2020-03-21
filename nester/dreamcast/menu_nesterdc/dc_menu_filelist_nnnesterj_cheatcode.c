#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_filelist.h"
#include "nes/nnnesterj_cheat.h"
#include "dc_menu_filelist_nnnesterj_cheatcode.h"

#include "dc_vmu.h"

#define DC_MENU_FILELIST_NNNESTERJ_CHEATCODE_ITEMINFO_SIZE 128
static dc_menu_filelist_iteminfo_t iteminfo[DC_MENU_FILELIST_NNNESTERJ_CHEATCODE_ITEMINFO_SIZE];


/* ============================================================ */
static void
setup_iteminfo(nes_nnnesterj_cheatinfo_t *nnc, int cheatinfo_size)
{
  int i;
  dc_menu_filelist_iteminfo_t *p;
  
  memset (iteminfo, 0, sizeof(iteminfo));
  
  p = iteminfo;
  for (i = 0; i < cheatinfo_size; ++i)
  {
    p->path[0] = '\0';
    sprintf(p->text, "%s", nnc->cheatinfo.comment);
    p->type = FILETYPE_FILE;
    p->selected = nnc->cheatinfo.check_flag;
    p->filesize = 0;
    
    ++p;
    ++nnc;
  }
  p->type = FILETYPE_NULL;
}


static dc_menu_filelist_result_type_t
dc_menu_filelist_nnnesterj_cheatcode_keyfunc(dc_menu_filelist_global_dirstatus_t *dirstatus)
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


void
dc_menu_filelist_nnnesterj_cheatcode (nes_nnnesterj_cheatinfo_t *nnc, int cheatinfo_size, dc_pvr_bgfunc_t bgfunc)
{
  dc_menu_filelist_info_t info;
  dc_menu_filelist_result_type_t result_type;
  dc_menu_filelist_iteminfo_t *p;
  
  dc_menu_filelist_display_nowloading (bgfunc);
  
  setup_iteminfo (nnc, cheatinfo_size);
  
  dc_menu_filelist_info_init (&info, iteminfo, "/nnnesterj_cheatcode");
  
  info.title = "Select Working Cheat Code";
  info.foot = "Press L to Select / Press START to Exit Menu";
  info.bgfunc = bgfunc;
  info.selectable = 1;
  info.is_display_filesize = 0;
  info.keyfunc = dc_menu_filelist_nnnesterj_cheatcode_keyfunc;
  
  result_type = dc_menu_filelist_common (&info);
  
  if (result_type == FILELIST_FINISH)
  {
    p = iteminfo;
    while (p->type != FILETYPE_NULL)
    {
      nnc->cheatinfo.check_flag = p->selected;
      
      ++p;
      ++nnc;
    }
  }
}


