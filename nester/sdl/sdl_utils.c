#include <stdio.h>
#include "sdl_utils.h"


int
load_bmp (uint16 *raw, char *filename)
{
  FILE *fp = NULL;
  uint8 buffer[320 * 240 * 3];
  uint8 *bmp;
  int x, y;
  uint16 *p;
  
  fp = fopen(filename, "r");
  if (!fp) 
  {
    perror(filename);
    goto error;
  }
  
  fseek(fp, 54, SEEK_SET);
  if (fread(buffer, sizeof(buffer), 1, fp) != 1) 
    goto error;
  
  fclose(fp);
  
  bmp = buffer;
  for (y = 0; y < 240; ++y)
  {
    p = raw + 320 * (239 - y);
    for (x = 0; x < 320; ++x)
    {
      uint16 r, g, b;
      
      b = *bmp++ * 32 / 256;
      g = *bmp++ * 64 / 256;
      r = *bmp++ * 32 / 256;
      
      *p++ = (r << 11) | (g << 5) | (b << 0);
    }
  }
  
  return 0;
  
error:
  if (fp) fclose(fp);
  memset (raw, 0, 320 * 240 * 2);
  fprintf(stderr, "something error in load_bmp\n");
  return -1;
}
