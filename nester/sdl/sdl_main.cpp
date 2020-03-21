#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"
#include "nes.h"
#include "null_sound_mgr.h"
#include "sdl_sound_mgr.h"
#include "sdl_saving.h"
#include "sdl_gg.h"


static uint32 frame_count = 0;
static volatile int rest_frames = 0;
static bool exit_game_loop = false;
static bool interrupt_game_loop = false;
static SDL_Surface *screen;

bool skip_sound = false;
static bool draw_pattern = true;

char nes_homedir[256];

uint16 
get_nesscreen_pixel_color(int x, int y)
{
  /* NOT SUPPORTED */
  return 0; 
}


uint8
keymap_sdl2dc[256];

static void
keymap_sdl2dc_init()
{
  int i;
  for (i = 0; i < 256; ++i)
    keymap_sdl2dc[i] = NES_device_kbd::NES_KEY_VoidSymbol;
  
  for (i = 0; i < 26; ++i)
    keymap_sdl2dc[SDLK_a + i] = NES_device_kbd::NES_KEY_A + i;
  
  for (i = 0; i < 10; ++i)
    keymap_sdl2dc[SDLK_0 + i] = NES_device_kbd::NES_KEY_0 + i;
}


void
draw_pattern_table(NES *emu)
{
  uint8 *p;
  uint16 *base;
  uint16 *buf;
  uint32 pattern1;
  uint32 pattern2;
  int xx, yy, x, y;
  int bank;
  int i;
  uint16* pattern_palette = emu->ppu_bg_pal_color ();
  
  if (SDL_MUSTLOCK (screen)) SDL_LockSurface (screen);
  
  base = (uint16 *)(screen->pixels) + 320 * 240;
  for (i = 0; i < 320; ++i)
  {
    *base++ = 0xffff;
  }
  
  for (bank = 0; bank < 8; ++bank)
  {
    p = emu->ppu_vram_banks(bank);
    
    for (yy = 0; yy < 2; ++yy)
    {
      for (xx = 0; xx < 32; ++xx)
      {
	for (y = 0; y < 8; ++y)
	{
	  pattern1 = (*(p + 8) & 0xaa) | ((*p >> 1) & 0x55);
	  pattern2 = ((*(p + 8) << 1) & 0xaa) | (*p & 0x55);
	  ++p;
	  
	  buf = base + 32 + (bank * 320 * 16) + (yy * 320 * 8) + (xx * 8) + (y * 320);
	  
	  buf += 8;
	  *(--buf) = pattern_palette[(pattern2 & 0x03)];
	  *(--buf) = pattern_palette[(pattern1 & 0x03)];
	  pattern2 >>= 2;
	  pattern1 >>= 2;
	  
	  *(--buf) = pattern_palette[(pattern2 & 0x03)];
	  *(--buf) = pattern_palette[(pattern1 & 0x03)];
	  pattern2 >>= 2;
	  pattern1 >>= 2;
	  
	  *(--buf) = pattern_palette[(pattern2 & 0x03)];
	  *(--buf) = pattern_palette[(pattern1 & 0x03)];
	  pattern2 >>= 2;
	  pattern1 >>= 2;
	  
	  *(--buf) = pattern_palette[(pattern2 & 0x03)];
	  *(--buf) = pattern_palette[(pattern1 & 0x03)];
	}
	p += 8;
      }
    }
  }
  
finish:
  
  if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);
}


static void
wait_until_release_button (SDLKey key)
{
  uint8 *keys;
  
  SDL_PumpEvents ();
  keys = SDL_GetKeyState (0);
  while (keys[key])
  {
    SDL_Delay (50);
    SDL_PumpEvents ();
    keys = SDL_GetKeyState (0);
  }
}


static void
poll_input(NES *emu)
{
  static int pad_num = 0;
  static int snap_num = 0;
  uint8 *keys; 
  NES_pad *pad = emu->get_pad (pad_num);
  
  SDL_PumpEvents ();
  keys = SDL_GetKeyState (0);
  
  if (keys[SDLK_ESCAPE])
  {
    exit_game_loop = true;
    interrupt_game_loop = true;
  }
  
  if (keys[SDLK_r] && keys[SDLK_LSHIFT])
  {
    emu->reset ();
    LOG("reset");
    wait_until_release_button (SDLK_r);
  }
  
  if (keys[SDLK_p])
  {
    LOG ("pause");
    wait_until_release_button (SDLK_p);
    
    while (!keys[SDLK_p])
    {
      SDL_Delay (10);
      SDL_PumpEvents ();
      keys = SDL_GetKeyState (0);
    }
    
    LOG ("quit pause");
    wait_until_release_button (SDLK_p);
  }
  
  if (keys[SDLK_1])
  {
    emu->SetDiskSide (0x01);
    LOG("set diskside 1A");
    wait_until_release_button (SDLK_1);
  }
  
  if (keys[SDLK_2])
  {
    emu->SetDiskSide (0x02);
    LOG("set diskside 1B");
    wait_until_release_button (SDLK_2);
  }
  
  if (keys[SDLK_3])
  {
    emu->SetDiskSide (0x03);
    LOG("set diskside 2A");
    wait_until_release_button (SDLK_3);
  }
  
  if (keys[SDLK_4])
  {
    emu->SetDiskSide (0x04);
    LOG("set diskside 2B");
    wait_until_release_button (SDLK_4);
  }
  
  if (keys[SDLK_5])
  {
    char filename[512];
    
    snprintf(filename, sizeof(filename),
	     "%s/snap/%s%d", 
             nes_homedir,
	     emu->getROMname (),
	     snap_num);
    if (emu->saveState(filename))
    {
      LOG("snapsave to");
      LOG(filename);
    }
    wait_until_release_button (SDLK_5);
  }
  
  if (keys[SDLK_7])
  {
    char filename[512];
    
    snprintf(filename, sizeof(filename),
	     "%s/snap/%s%d", 
             nes_homedir,
	     emu->getROMname (), 
	     snap_num);
    if (emu->loadState(filename))
    {
      LOG("snapload from");
      LOG(filename);
    }
    wait_until_release_button (SDLK_7);
  }
  
  if (keys[SDLK_w])
  {
    if (snap_num == 0)
      snap_num = 8;
    else
      --snap_num;
    
    printf("snap_num = %d\n", snap_num);
    wait_until_release_button (SDLK_w);
  }
  
  if (keys[SDLK_e])
  {
    if (snap_num == 8)
      snap_num = 0;
    else
      ++snap_num;
    
    printf("snap_num = %d\n", snap_num);
    wait_until_release_button (SDLK_e);
  }
  
  if (keys[SDLK_EQUALS])
  {
    ++(emu->CYCLES_PER_LINE);
    
    printf("CYCLES_PER_LINE = %f\n", emu->CYCLES_PER_LINE);
    
    if (!keys[SDLK_LSHIFT])
      wait_until_release_button (SDLK_EQUALS);
  }
  if (keys[SDLK_MINUS])
  {
    --(emu->CYCLES_PER_LINE);
    
    printf("CYCLES_PER_LINE = %f\n", emu->CYCLES_PER_LINE);
    
    if (!keys[SDLK_LSHIFT])
      wait_until_release_button (SDLK_MINUS);
  }
  if (keys[SDLK_BACKSLASH])
  {
    pad_num = (pad_num + 1) & 0x03;
    
    printf("pad_num = %d\n", pad_num);
    
    wait_until_release_button (SDLK_BACKSLASH);
  }
  if (keys[SDLK_x])
  {
    draw_pattern = !draw_pattern;
    
    printf("draw_pattern = %d\n", draw_pattern);
    
    wait_until_release_button (SDLK_x);
  }
  
  {
    int key;

#if 1
    emu->NES_kbd.clear_register ();
#endif
    
    for (key = SDLK_FIRST; key < SDLK_LAST; ++key)
      if (keys[key]) emu->NES_kbd.setkey(keymap_sdl2dc[key]);

#if 0    
    emu->NES_kbd.setkey(NES_device_kbd::NES_KEY_M);
#endif
  }
  
  skip_sound = keys[SDLK_SPACE];
  
  pad->nes_UP = keys[SDLK_k];
  pad->nes_DOWN = keys[SDLK_j];
  pad->nes_LEFT = keys[SDLK_h];
  pad->nes_RIGHT = keys[SDLK_l];
  pad->nes_SELECT = keys[SDLK_SEMICOLON];
  pad->nes_START = keys[SDLK_QUOTE];
  pad->nes_B = keys[SDLK_s];
  pad->nes_A = keys[SDLK_f];
}


void
do_game (const char *filename, const char *geniefile, const char *nnnesterj_cheatfile) 
{
  NES *emu = NULL;
  sound_mgr *snd_mgr = NULL;
  
#ifdef __UNIX__
  snd_mgr = new sdl_sound_mgr ();
#else
  snd_mgr = new null_sound_mgr ();
#endif
  if (!snd_mgr) goto finish;
  if (!(snd_mgr->initialize ())) goto finish;
  
  emu = new NES (snd_mgr);
  if (!emu) goto finish;
  snprintf(emu->disksys_rom_filename, sizeof(emu->disksys_rom_filename), "%s/disksys.rom", nes_homedir);
  if (!(emu->initialize (filename))) goto finish;
  
  sram_load (emu);
  disk_load (emu);
  
  exit_game_loop = false;
  interrupt_game_loop = false;
  frame_count = 0;

  if (geniefile)
    load_genie_from_file (geniefile, emu);
  
  if (nnnesterj_cheatfile)
    emu->load_nnnesterj_cheat (nnnesterj_cheatfile);
  
  printf("mapper = %d\n", emu->get_mapper_num());
  
  while (!exit_game_loop)
  {
    while (!interrupt_game_loop)
    {
#ifdef __SDL_DEBUG__
      if (draw_pattern)
	draw_pattern_table (emu);
#endif
      
      if (SDL_MUSTLOCK (screen)) SDL_LockSurface (screen);
      emu->emulate_frame ((uint16 *)(screen->pixels));
      
      {
	int y;
	uint16 *p;
	
	for (y = 0; y < 224; ++y)
	{
	  p = (uint16 *)(screen->pixels) + 320 * y;
	  
	  *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
	  *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
	  
	  p += 256;
	  
	  *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
	  *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
	}
      }
      if (SDL_MUSTLOCK (screen)) SDL_UnlockSurface (screen);
      
      if (skip_sound)
	emu->emulate_frame_skip();
      
      ++frame_count;
      
      SDL_Flip(screen);
      
      poll_input(emu);
    }
  }
  
  sram_save (emu);
  disk_save (emu);
  
finish:
  
  if (emu) delete (emu);
  if (snd_mgr) delete (snd_mgr);
}


int
init_SDL_video ()
{
  const SDL_VideoInfo *vinf;
  int flags = 0;
  
  vinf = SDL_GetVideoInfo();
  
  if (vinf->hw_available)
    flags |= SDL_HWSURFACE;
  flags |= SDL_DOUBLEBUF;

#ifdef __SDL_DEBUG__
  screen = SDL_SetVideoMode (320, 480, 16, flags);
#else
  screen = SDL_SetVideoMode (320, 240, 16, flags);
#endif
  if (!screen) return 0;
  
  SDL_WM_SetCaption ("NesterDC for SDL", "NesterDC for SDL");
  
  return 1;
}


int
shutdown_SDL_video ()
{
  SDL_Quit ();
}


#if 1
#include "testing.cpp"
#endif


extern "C" int
main(int argc, char **argv)
{
  char module_dirname[512];
  char etc_dirname[512];
  mode_t mode;
  int i;
  
  if (argc == 1) 
  {
    fprintf(stderr, "Usage: nester romfile\n");
    exit(1);
  }
  
  keymap_sdl2dc_init ();
  
  if(SDL_Init(SDL_INIT_VIDEO)) //|SDL_INIT_AUDIO)==-1)
  {
    printf("Could not initialize SDL: %s.\n", SDL_GetError());
    return(-1);
  }
  
  snprintf(nes_homedir, sizeof(nes_homedir), "%s/.nester", getenv("HOME"));
  
  mode = S_IRWXU;
  mkdir(module_dirname, mode);
  snprintf(etc_dirname, sizeof(etc_dirname),
	   "%s/sav", module_dirname);
  mkdir(etc_dirname, mode);
  snprintf(etc_dirname, sizeof(etc_dirname),
	   "%s/snap", module_dirname);
  mkdir(etc_dirname, mode);
  
  if (!init_SDL_video ())
  {
    fprintf(stderr, "error in SDL video init\n");
    exit(1);
  }
  
#if 0
  test_all ();
#endif
  
  if (argc == 2)
    do_game (argv[1], NULL, NULL);
  else
  {
#if 0
    do_game (argv[1], argv[2], NULL);
#else
    do_game (argv[1], NULL, argv[2]);
#endif
  }
  
  shutdown_SDL_video ();
  return 0;
}


