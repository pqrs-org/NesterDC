#ifndef _SDL_SOUND_MGR_H_
#define _SDL_SOUND_MGR_H_

#include "types.h"
#include "sound_mgr.h"
#include <SDL.h>

#define SAMPLE_RATE 44100
#define SOUND_BUF_LEN (735 * 2)
#define SAMPLE_BITS 16

class sdl_sound_mgr : public sound_mgr
{
public:
  sdl_sound_mgr();
  ~sdl_sound_mgr();
  
  void reset() {}
  
  // lock down for a period of inactivity
  void freeze() {}
  void thaw() {}
  
  void clear_buffer();
  
  boolean lock(sound_buf_pos which, void** buf, uint32* buf_len);
  void unlock();
  
  int get_sample_rate() { return SAMPLE_RATE; }
  int get_sample_bits() { return SAMPLE_BITS; }
  
  // returns SOUND_BUF_LOW or SOUND_BUF_HIGH
  sound_buf_pos get_currently_playing_half();
  
  boolean IsNull() { return FALSE; }
  
 private:
  int dspfd;
  int buffer_locked;
  uint8 buffer[8840];
};

#endif


