#ifndef _DC_SOUND_MGR_H_
#define _DC_SOUND_MGR_H_

#include "types.h"
#include "sound_mgr.h"
#include "dc_sound.h"

/* SOUND_BUF_LEN = 2 * (rate * buffer_length_in_frames / 60.0) * (bit / 8) */
/* (format "%x" ((lambda (x) (* 2 (/ (* 44100 x) 60) (/ 16 8))) 3)) */
/* NOTE: AICA require 32-bit (or higher) alignment. */
#define SAMPLE_RATE 44100
#define SAMPLE_BITS 16
#define SOUND_BUF_LEN 0x2000

class dc_sound_mgr : public sound_mgr
{
public:
  ~dc_sound_mgr ();
  bool initialize ();
  
  void reset() {}
  
  void freeze() {}
  void thaw() {}
  
  void clear_buffer();
  boolean lock(sound_buf_pos which, void** buf, uint32* buf_len);
  void unlock();
  
  int get_sample_rate() { return SAMPLE_RATE; }
  int get_sample_bits() { return SAMPLE_BITS; }
  
  sound_buf_pos get_currently_playing_half();
  
  boolean IsNull() { return FALSE; }
  
private:
  uint8 buffer[SOUND_BUF_LEN / 2] __attribute__ ((aligned (32)));
  bool buffer_locked;
  int curbuffer;
};

#endif


