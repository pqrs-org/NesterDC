#include "dc_utils.h"
#include "dc_menu_common.h"
#include "dc_menu_filelist.h"
#include "dc_menu_filelist_cdfs.h"

#define DC_MENU_FILELIST_CDFS_ITEMINFO_SIZE 1024
static dc_menu_filelist_iteminfo_t iteminfo[DC_MENU_FILELIST_CDFS_ITEMINFO_SIZE];

static void
read_directory (const char *path)
{
  int i;
  file_t d; 
  dirent_t *de;
  dc_menu_filelist_iteminfo_t *p;
  
  memset(iteminfo, 0, sizeof(iteminfo));
  
  d = fs_open(path, O_RDONLY | O_DIR);
  if (!d) return;
  
  p = iteminfo;
  de = fs_readdir (d);
  for (i = 0; i < DC_MENU_FILELIST_CDFS_ITEMINFO_SIZE; ++i)
  {
    if (de)
    {
      sprintf (p->path, "%s/%s", path, de->name); 
      if (de->size == -1)
      {
        sprintf (p->text, "%s/", de->name);
        p->type = FILETYPE_DIR;
      }
      else
      {
        sprintf (p->text, "%s", de->name);
        p->type = FILETYPE_FILE;
      }
      p->selected = 0;
      p->filesize = de->size;
      
      de = fs_readdir (d);
    }
    else
      p->type = FILETYPE_NULL;
    
    ++p;
  }
  fs_close (d);
  
  /* make last item as guard */
  iteminfo[DC_MENU_FILELIST_CDFS_ITEMINFO_SIZE - 1].type = FILETYPE_NULL;
}


static dc_menu_filelist_result_type_t
dc_menu_filelist_cdfs_keyfunc(dc_menu_filelist_global_dirstatus_t *dirstatus)
{
  dc_menu_controller_status_t *p;
  
  p = dc_menu_controller_status;
  while (p->dev)
  {
    if (dc_menu_keypress(p->START) &&
        iteminfo[dirstatus->cur_pos].type == FILETYPE_FILE)
      return FILELIST_FINISH;
    
    if (dc_menu_keypress(p->A) &&
        iteminfo[dirstatus->cur_pos].type == FILETYPE_DIR)
      return FILELIST_FINISH;
    
    if (dc_menu_keypress(p->Y))
      return FILELIST_PARENTDIR;
    
    if (dc_menu_keypress(p->B))
      dc_menu_filelist_global_config_init();
    
    if (dc_menu_keypress(p->analog_up))
      --(dc_menu_filelist_global_config.cursor_wait);
    if (dc_menu_keypress(p->analog_down))
      ++(dc_menu_filelist_global_config.cursor_wait);
    if (dc_menu_keypress(p->analog_left))
      --(dc_menu_filelist_global_config.pageup_wait);
    if (dc_menu_keypress(p->analog_right))
      ++(dc_menu_filelist_global_config.pageup_wait);
    
    ++p;
  }
  
  return FILELIST_NOTHING;
}


void
dc_menu_filelist_cdfs (char *result, int result_len, const char *path, dc_pvr_bgfunc_t bgfunc)
{
  char new_path[DC_MENU_FILELIST_MAX_PATH_LEN];
  dc_menu_filelist_info_t info;
  dc_menu_filelist_result_type_t result_type;
  int cur_pos;
  
  dc_menu_filelist_display_nowloading (bgfunc);
  
  /* reset GDROM cache for Disk-Swapping */
  iso_ioctl (0, NULL, 0);
  
  sprintf(new_path, "%s", path);
  for (;;)
  {
    read_directory (new_path);
    
    dc_menu_filelist_info_init (&info, iteminfo, new_path);
    
    info.title = "File Select";
    info.foot = NULL;
    info.bgfunc = bgfunc;
    info.selectable = 0;
    info.is_display_filesize = 0;
    info.keyfunc = dc_menu_filelist_cdfs_keyfunc;
    
    result_type = dc_menu_filelist_common (&info);
    
    switch (result_type)
    {
      case FILELIST_FINISH:
        cur_pos = (info.dirstatus)->cur_pos;
        
        if (iteminfo[cur_pos].type == FILETYPE_DIR)
          sprintf(new_path, "%s", iteminfo[cur_pos].path);
        else if (iteminfo[cur_pos].type == FILETYPE_FILE)
        {
          sprintf (result, "%s", iteminfo[(info.dirstatus)->cur_pos].path);
          return;
        }
        break;
        
      case FILELIST_PARENTDIR:
        if (strcmp(new_path, "/cd"))
        {
          char *dirp = strrchr(new_path, '/');
          if (dirp) *dirp = '\0';
        }
        break;
        
      case FILELIST_ABORT:
        result[0] = '\0';
        return;
        
      case FILELIST_NOTHING:
        break;
    }
  }
}


