static void
test_nes ()
{
  int i;
  NES_pad *pad = NULL;
  sound_mgr *snd_mgr = NULL;
  char buf[128];
  
  for (i = 0; i < 0x1000; ++i)
  {
    pad = new NES_pad ();
    delete (pad);
  }
}


#include "bzlib.h"

static void
test_bzip2 ()
{
  char buf[128];
  char buf1[0x4000];
  char buf2[0x4000];
  int buf1_len = sizeof(buf1);
  int buf2_len = sizeof(buf2);
  int i;
  
  for (i = 0; i < sizeof(buf1); ++i)
  {
    if (i & 0x01)
      buf1[i] = buf1[i - 1] >> 1;
    else
      buf1[i] = (i >> 1) ^ 0xedb88320;
  }
  
  for (i = 0; i < 0x10; ++i)
  {
    buf2_len = sizeof(buf2);
    if (BZ2_bzBuffToBuffCompress (buf2, (unsigned int *)&buf2_len, 
				  buf1, sizeof(buf1), 
				  9, 0, 0) != BZ_OK)
    {
      fprintf (stderr, "compress error\n");
    }
    
    buf1_len = sizeof(buf1);
    if (BZ2_bzBuffToBuffDecompress (buf1, (unsigned int *)&buf1_len,
				    buf2, buf2_len,
				    0, 0) != BZ_OK)
    {
      fprintf (stderr, "decompress error\n");
    }
  }
}


#include "sdl_utils.h"

static void
test_utils ()
{
  uint16 image[320 * 240];
  int i;
  
  load_bmp (image, "menu.bmp");
  
  if (SDL_MUSTLOCK (screen)) SDL_LockSurface (screen);
  memcpy(screen->pixels, image, 320 * 240 * 2);
  if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);
  
  SDL_Flip(screen);
  
  sleep(1);
}


static void
test_all ()
{
  test_nes ();
  //test_bzip2 ();
  test_utils ();
}
