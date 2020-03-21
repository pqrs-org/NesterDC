#include "types.h"
#include "sdl_sound_mgr.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <sys/soundcard.h>


static int
open_dsp ()
{
  int x;
  int dspfd = -1;
  
  dspfd = open("/dev/dsp", O_WRONLY);
  if (dspfd < 0)
  {
    LOG("can't open /dev/dsp");
    goto error;
  }
  
#ifdef HOST_LITTLE_ENDIAN
  x = AFMT_S16_LE;
#else
  x = AFMT_S16_BE;
#endif
  if (ioctl (dspfd, SNDCTL_DSP_SETFMT, &x) < 0)
  {
    LOG("can't set format to 16-bit");
    goto error;
  }
  
  x = 9 | 8 << 16;
  if (ioctl (dspfd, SNDCTL_DSP_SETFRAGMENT, &x) < 0)
  {
    LOG("can't set fragment size");
    goto error;
  }
  
  x = 0;
  if (ioctl (dspfd, SNDCTL_DSP_STEREO, &x) < 0)
  {
    LOG("can't set to stereo");
    goto error;
  }
  
  x = SAMPLE_RATE;
  if (ioctl (dspfd, SNDCTL_DSP_SPEED, &x) < 0)
  {
    LOG("can't set sample rate");
    goto error;
  }
  
  LOG("sound init done");
  return dspfd;
  
error:
  if (dspfd >= 0) close(dspfd);
  return -1;
}


sdl_sound_mgr::sdl_sound_mgr()
{
  buffer_locked = false;
  dspfd = open_dsp ();
  memset (buffer, 0, sizeof(buffer));
}


sdl_sound_mgr::~sdl_sound_mgr()
{
  LOG("sound terminate");
  if (dspfd >= 0) close(dspfd);
  dspfd = -1;
}


void
sdl_sound_mgr::clear_buffer()
{
}


boolean 
sdl_sound_mgr::lock(sound_buf_pos which, void** buf, uint32* buf_len)
{
  if (buffer_locked) return false;
  
  buffer_locked = true;
  *buf = buffer;
  *buf_len = SOUND_BUF_LEN;
  return true;
}


extern bool skip_sound;

void
sdl_sound_mgr::unlock()
{
  static int skip = 0;
  
  if (!buffer_locked) return;
  
  buffer_locked = false;
  
  if (skip_sound)
    skip = (skip + 1) & 0x3;
  else
    skip = 0;
  
  if (dspfd >= 0 && !skip)
    write(dspfd, buffer, SOUND_BUF_LEN);
}


/* return the alternation of SOUND_BUF_LOW and SOUND_BUF_HIGH,
   because of NES_APU::DoFrame implement. */
sound_mgr::sound_buf_pos
sdl_sound_mgr::get_currently_playing_half()
{
  static sound_mgr::sound_buf_pos last = SOUND_BUF_HIGH;
  
  if (last == SOUND_BUF_HIGH)
    last = SOUND_BUF_LOW;
  else
    last = SOUND_BUF_HIGH;
  
  return last;
}


