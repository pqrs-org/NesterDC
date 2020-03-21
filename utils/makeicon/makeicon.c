#include <stdio.h>

typedef	unsigned char	uint8;
typedef	unsigned short	uint16;
typedef	unsigned int	uint32;

int
main()
{
  uint8 palette[16 * 4];
  uint16 new_palette[16];
  uint8 *p;
  uint8 pixel[32 * 32 / 2];
  int i;
  
  fseek(stdin, 54, SEEK_SET);
  fread(palette, sizeof(palette), 1, stdin);
  
  p = palette;
  for (i = 0; i < 16; ++i)
  {
    uint16 r, g, b;
    
    b = *p++ * 32 / 256;
    g = *p++ * 64 / 256;
    r = *p++ * 32 / 256;
    *p++;
    
    new_palette[i] = (r << 11) | (g << 5) | (b << 0);
  }
  fwrite(new_palette, sizeof(new_palette), 1, stdout);
  
  fread(pixel, sizeof(pixel), 1, stdin);
#if 0
  for (i = 0; i < sizeof(pixel); ++i)
  {
    if ((pixel[i] & 0xf0) == 0)
      pixel[i] |= 0xa0;
    if ((pixel[i] & 0x0f) == 0)
      pixel[i] |= 0x0a;
  }
#endif
  
  for (i = 0; i < 32; ++i)
    fwrite(pixel + 16 * (31 - i), 16, 1, stdout);
}
