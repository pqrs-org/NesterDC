#include "types.h"
#include "dc_sound_mgr.h"
#include "dc_utils.h"


bool 
dc_sound_mgr::initialize ()
{
  buffer_locked = false;
  memset(buffer, 0, sizeof(buffer));
  clear_buffer ();
  dc_sound_init (SAMPLE_RATE, SAMPLE_BITS, SOUND_BUF_LEN);
  
  return true;
}


dc_sound_mgr::~dc_sound_mgr()
{
  dc_sound_shutdown ();
}


void
dc_sound_mgr::clear_buffer()
{
  spu_memset (dc_sound_get_baseaddr (), 0, SOUND_BUF_LEN);
}


boolean 
dc_sound_mgr::lock(sound_buf_pos which, void** buf, uint32* buf_len)
{
  if (buffer_locked) return false;
  
  buffer_locked = true;
  curbuffer = (which == SOUND_BUF_HIGH);
  *buf = buffer;
  *buf_len = sizeof(buffer);
  return true;
}


void
dc_sound_mgr::unlock()
{
  if (!buffer_locked) return;
  
  buffer_locked = false;
  spu_memload (dc_sound_get_baseaddr () + sizeof(buffer) * curbuffer, 
	       buffer, sizeof(buffer));
}


/* return the alternation of SOUND_BUF_LOW and SOUND_BUF_HIGH,
   because of NES_APU::DoFrame implement. */
sound_mgr::sound_buf_pos
dc_sound_mgr::get_currently_playing_half()
{
  uint32 val = dc_sound_get_position () * (SAMPLE_BITS / 8);
  
  if (val < SOUND_BUF_LEN / 2)
    return SOUND_BUF_LOW;
  else
    return SOUND_BUF_HIGH;
}


