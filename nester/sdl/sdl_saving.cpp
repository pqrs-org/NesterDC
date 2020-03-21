#include "types.h"
#include "nes.h"
#include "debug.h"
#include "sdl_saving.h"

#include <stdio.h>

extern char nes_homedir[];

static void
set_savefilename(char *str, int len, NES *emu)
{
  snprintf(str, len, 
	   "%s/sav/%s", 
           nes_homedir, 
	   emu->getROMname());
}


bool
sram_save (NES *emu)
{
  FILE *fp = NULL;
  char filename[512];
  
  if (!emu->has_sram ()) return true;
  if (emu->is_invalid_sram ()) return true;
  
  set_savefilename(filename, sizeof(filename), emu);
  fp = fopen(filename, "w");
  if (!fp) 
    goto error;
  
  if (fwrite(emu->get_SaveRAM (), NES::sram_buflen, 1, fp) != 1)
    goto error;
  
  fclose(fp);
  
  LOG("save done");
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


bool
sram_load (NES *emu)
{
  FILE *fp = NULL;
  char filename[512];
  uint8 sram[NES::sram_buflen];
  int sram_len;
  int nread;
  
  if (!emu->has_sram ()) return true;
  
  set_savefilename(filename, sizeof(filename), emu);
  fp = fopen(filename, "r");
  if (!fp)
    goto error;
  
  nread = fread(sram, 1, sizeof(sram), fp);
  if (nread < 0)
    goto error;
  
  emu->set_SaveRAM (sram, nread);
  fclose(fp);
  
  LOG("load done");
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


bool 
disk_save (NES *emu)
{
  uint8 buf[emu->fds_save_buflen];
  char filename[512];
  FILE *fp = NULL;
  
  // must not save before load disk image to disk[] in mapper reset
  if(emu->GetDiskData(0) != 0x01)
    return false;
  
  emu->make_savedata(buf);
  set_savefilename(filename, sizeof(filename), emu);
  fp = fopen(filename, "w");
  if (!fp)
    goto error;
  
  if (fwrite(buf, sizeof(buf), 1, fp) != 1)
    goto error;
  
  fclose(fp);
  
  LOG ("save done");
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


bool
disk_load (NES *emu)
{
  uint8 buf[emu->fds_save_buflen];
  char filename[512];
  FILE *fp = NULL;
  
  if (emu->GetDiskData(0) != 0x01)
    return false;
  
  set_savefilename(filename, sizeof(filename), emu);
  fp = fopen(filename, "r");
  if (!fp)
    goto error;
  
  fread(buf, sizeof(buf), 1, fp);
  fclose(fp);
  
  emu->restore_savedata (buf);
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


