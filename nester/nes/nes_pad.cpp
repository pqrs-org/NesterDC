#include "nes.h"
#include "nes_pad.h"

enum NES_BUTTON
{
  NES_A,
  NES_B,
  NES_SELECT,
  NES_START,
  NES_UP, 
  NES_DOWN,
  NES_LEFT,
  NES_RIGHT,
};

uint8
NES_pad::get_state()
{
  uint8 rv = 0x00;
  
  rv |= (nes_A << NES_A);
  rv |= (nes_B << NES_B);
  rv |= (nes_SELECT << NES_SELECT);
  rv |= (nes_START << NES_START);
  rv |= (nes_UP << NES_UP);
  rv |= (nes_DOWN << NES_DOWN);
  rv |= (nes_LEFT << NES_LEFT);
  rv |= (nes_RIGHT << NES_RIGHT);
  
  return rv;
}


