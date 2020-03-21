#ifndef _DC_MENU_FILELIST_H_
#define _DC_MENU_FILELIST_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================ */
/* global settings */
typedef struct {
  uint8 cursor_wait;
  uint8 pageup_wait;
} dc_menu_filelist_global_config_t;

extern dc_menu_filelist_global_config_t dc_menu_filelist_global_config;
void dc_menu_filelist_global_config_init();

#define DC_MENU_FILELIST_MAX_PATH_LEN 256
typedef struct {
  char path[DC_MENU_FILELIST_MAX_PATH_LEN];
  int num_files;
  int base_pos;
  int cur_pos;
} dc_menu_filelist_global_dirstatus_t;

/* need 2^x */
#define DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE 64
extern dc_menu_filelist_global_dirstatus_t dc_menu_filelist_global_dirstatus[DC_MENU_FILELIST_GLOBAL_DIRSTATUS_SIZE];

void dc_menu_filelist_init ();


/* ============================================================ */
/* local settings (per each filelist menu) */
typedef enum {
  FILETYPE_NULL,
  FILETYPE_DIR,
  FILETYPE_FILE,
} dc_menu_filelist_itemtype_t;

#define DC_MENU_FILELIST_MAX_TEXT_LEN 80
typedef struct {
  char path[DC_MENU_FILELIST_MAX_PATH_LEN];
  char text[DC_MENU_FILELIST_MAX_TEXT_LEN];
  dc_menu_filelist_itemtype_t type;
  int selected;
  int filesize;
} dc_menu_filelist_iteminfo_t;

/* if keyfunc return 1, then exit menu. */
typedef enum 
{
  FILELIST_NOTHING,
  FILELIST_FINISH,
  FILELIST_PARENTDIR,
  FILELIST_ABORT,
} dc_menu_filelist_result_type_t;

typedef dc_menu_filelist_result_type_t (*dc_menu_filelist_keyfunc)(dc_menu_filelist_global_dirstatus_t *);

typedef struct _dc_menu_filelist_info {
  /* 
   * set by user
   */
  const char *title;
  const char *foot;
  dc_pvr_bgfunc_t bgfunc;
  
  int selectable;
  int is_display_filesize;
  dc_menu_filelist_keyfunc keyfunc;
  
  /* 
   * set by dc_menu_filelist_info_init
   */
  dc_menu_filelist_iteminfo_t *iteminfo;
  dc_menu_filelist_global_dirstatus_t *dirstatus;
  
} dc_menu_filelist_info_t;

void dc_menu_filelist_info_init(dc_menu_filelist_info_t *info, dc_menu_filelist_iteminfo_t *iteminfo, const char *path);

#define DRAW_LINES 10

void dc_menu_filelist_display_nowloading(dc_pvr_bgfunc_t bgfunc);
dc_menu_filelist_result_type_t dc_menu_filelist_common (dc_menu_filelist_info_t *info);

#ifdef __cplusplus
}
#endif

#endif


