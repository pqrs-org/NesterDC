#include "kos.h"
#include "nes.h"
#include <stdio.h>
#include "stdlib.h"

#include "dc_utils.h"
#include "bzlib.h"
#include "dc_vmu.h"


static const uint8 nesterdc_icon[] = {
#include "icons/nesterdc-icon.h"
};

bool
sram_save_to_vmu (maple_device_t *dev, NES *emu)
{
  const uint8 *buf;
  uint buf_len;
  uint8 compressed_buf[NES::sram_buflen];
  uint compressed_len;
  char vmu_filename[512];
  char desc_long[512];
  
  if (!dev) return false;
  
  if (!emu->has_sram ()) return true;
  if (emu->is_invalid_sram ()) return true;
  
  buf = emu->get_SaveRAM ();
  buf_len = NES::sram_buflen;
  compressed_len = sizeof(compressed_buf);
  
  dc_pvr_font_output_message (NULL, "Saving SRAM...", NULL, NULL);
  
  if (BZ2_bzBuffToBuffCompress((char *)compressed_buf, 
			       (unsigned int*)&compressed_len, 
			       (char *)buf, buf_len, 1, 0, 0) != BZ_OK) 
  {
    return false;
  }
  
  sprintf(vmu_filename, "%s", emu->getROMcrc ());
  sprintf(desc_long, "NesterDC %s", emu->getROMname());
  if (ndc_vmu_save(compressed_buf, compressed_len, dev, 
		   vmu_filename, "NesterDC SRAM", desc_long, nesterdc_icon) < 0)
  {
    dc_pvr_font_output_message (NULL, "Save failed", NULL, NULL);
    timer_spin_sleep (1000);
    return false;
  }
  
  return true;
}


bool
sram_load_from_vmu (maple_device_t *dev, NES *emu)
{
  uint8 buf[NES::sram_buflen];
  uint32 buf_len;
  uint8 compressed_buf[NES::sram_buflen];
  uint32 compressed_len;
  char vmu_filename[512];
  
  if (!dev) return false;
  
  if (!emu->has_sram ()) return true;
  
  if (ndc_vmu_check_free_blocks (NULL, dev) < 18)
  {
    dc_pvr_font_output_message (NULL, "caution: VMU free blocks < 18", NULL, NULL);
    timer_spin_sleep (1000);
  }
  
  buf_len = sizeof(buf);
  compressed_len = sizeof(compressed_buf);
  
  dc_pvr_font_output_message (NULL, "Loading SRAM...", NULL, NULL);
  
  sprintf(vmu_filename, "%s", emu->getROMcrc ());
  if (ndc_vmu_load(compressed_buf, &compressed_len, dev, vmu_filename) == 0) 
  {
    if (BZ2_bzBuffToBuffDecompress((char *)buf, 
				   (unsigned int*)&buf_len, 
				   (char *)compressed_buf, 
				   compressed_len, 0, 0) != BZ_OK)
    {
      return false;
    }
  }
  else 
  {
    /* for compatibility */
    char old_filename[32];
    const char *romname = emu->getROMname();
    int i;
    int len;
    
    len = strlen(romname);
    
    for (i = 0; i < 11; ++i)
    {
      if (i < len)
	old_filename[i] = toupper(romname[i]);
      else
	old_filename[i] = ' ';
    }
    
    old_filename[11] = '&';
    old_filename[12] = '\0';
    
    if (ndc_vmu_load(buf, &buf_len, dev, old_filename) < 0)
    {
      dc_pvr_font_output_message (NULL, "No savefile in VMU", NULL, NULL);
      return false;
    }
  }
  
  emu->set_SaveRAM (buf, buf_len);
  return true;
}


bool
disk_save_to_vmu (maple_device_t *dev, NES *emu)
{
  uint8 buf[NES::fds_save_buflen];
  uint buf_len;
  uint8 compressed_buf[NES::fds_save_buflen];
  uint compressed_len;
  char vmu_filename[512];
  char desc_long[512];
  
  if (!dev) return false;
  
  // must not save before load disk image to disk[] in mapper reset
  if (emu->GetDiskData(0) != 0x01) return false;
  
  dc_pvr_font_output_message (NULL, "Saving Disk...", NULL, NULL);
  
  buf_len = sizeof(buf);
  compressed_len = sizeof(compressed_buf);
  
  emu->make_savedata(buf);
  if (BZ2_bzBuffToBuffCompress((char *)compressed_buf,
			       (unsigned int*)&compressed_len, 
			       (char *)buf, buf_len, 
			       1, 0, 0) != BZ_OK) 
  {
    return false;
  }
  
  sprintf(vmu_filename, "%s", emu->getROMcrc ());
  sprintf(desc_long, "NesterDC %s", emu->getROMname());
  if (ndc_vmu_save(compressed_buf, compressed_len, dev, 
		   vmu_filename, "NesterDC FDS", desc_long, nesterdc_icon) < 0)
    return false;
  
  return true;
}


bool
disk_load_from_vmu (maple_device_t *dev, NES *emu)
{
  uint8 buf[NES::fds_save_buflen];
  uint32 buf_len;
  uint8 compressed_buf[NES::fds_save_buflen];
  uint32 compressed_len;
  char vmu_filename[512];
  
  // must not load before load disk image to disk[] in mapper reset
  if(emu->GetDiskData(0) != 0x01)
    return false;
  
  dc_pvr_font_output_message (NULL, "Loading Disk...", NULL, NULL);
  
  buf_len = sizeof(buf);
  compressed_len = sizeof(compressed_buf);
  
  sprintf(vmu_filename, "%s", emu->getROMcrc ());
  if (ndc_vmu_load(compressed_buf, &compressed_len, dev, vmu_filename) < 0) 
    return false;
  
  if (BZ2_bzBuffToBuffDecompress((char *)buf, 
				 (unsigned int*)&buf_len, 
				 (char *)compressed_buf, 
				 compressed_len, 
				 0, 0) != BZ_OK)
  {
    return false;
  }
  
  emu->restore_savedata (buf);
  return true;
}


static bool
snss_save_to_vmu (maple_device_t *dev, NES *emu)
{
  FILE *fp = NULL;
  uint8 buf[NES::snss_buflen];
  uint buf_len;
  uint8 compressed_buf[NES::snss_buflen];
  uint compressed_len;
  char desc_long[512];
  char filename[512];
  
  if (!dev) goto error;
  
  fp = fopen("/md/quick", "r");
  if (!fp)
    goto error;
  fseek(fp, 0, SEEK_END);
  buf_len = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (fread(buf, buf_len, 1, fp) != 1)
    goto error;
  fclose(fp);
  fp = NULL;
  
  compressed_len = sizeof(compressed_buf);
  
  if (BZ2_bzBuffToBuffCompress((char *)compressed_buf, 
			       (unsigned int*)&compressed_len,
			       (char *)buf, 
			       buf_len, 
			       1, 0, 0) != BZ_OK)
  {
    goto error;
  }
  
  sprintf(desc_long, "NesterDC snap %s", emu->getROMname());
  sprintf(filename, "%s_SS", emu->getROMcrc());
  if (ndc_vmu_save(compressed_buf, compressed_len, dev, 
		   filename, "NesterDC Snap", desc_long, nesterdc_icon) < 0)
    goto error;
  
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


bool
snap_save_to_vmu (maple_device_t *dev, NES *emu)
{
  if (!(emu->saveState("/md/quick"))) 
    return false;
  
  if (!snss_save_to_vmu (dev, emu))
    return false;
  
  return true;
}


static bool
snss_load_from_vmu (maple_device_t *dev, NES *emu)
{
  uint8 compressed_buf[NES::snss_buflen];
  uint32 compressed_len;
  uint8 buf[NES::snss_buflen];
  size_t buf_len;
  char filename[512];
  FILE *fp = NULL;
  
  if (!dev) goto error;
  
  sprintf(filename, "%s_SS", emu->getROMcrc());
  compressed_len = sizeof(compressed_buf);
  buf_len = sizeof(buf);
  if (ndc_vmu_load(compressed_buf, &compressed_len, dev, filename) < 0)
    goto error;
  if (BZ2_bzBuffToBuffDecompress((char *)buf, (unsigned int*)&buf_len, 
				 (char *)compressed_buf, compressed_len,
				 0, 0) != BZ_OK)
    goto error;
  
  fp = fopen("/md/quick", "w");
  if (!fp)
    goto error;
  if (fwrite(buf, buf_len, 1, fp) != 1)
    goto error;
  fclose(fp);
  fp = NULL;
  
  return true;
  
error:
  if (fp) fclose(fp);
  return false;
}


bool
snap_load_from_vmu (maple_device_t *dev, NES *emu)
{
  if (!snss_load_from_vmu (dev, emu))
    return false;
  
  if (!emu->loadState("/md/quick"))
    return false;
  
  return true;
}



