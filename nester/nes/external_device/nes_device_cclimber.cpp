#include "nes.h"
#include "nes_device_cclimber.h"

void
NES_device_cclimber::WriteReg4016(NES *emu)
{ 
  uint32 b0, b1;
  
  b0 = emu->pad_bits[0] & 0x0f;
  b0 |= left_RIGHT << 4;
  b0 |= left_LEFT << 5;
  b0 |= left_UP << 6;
  b0 |= left_DOWN << 7;
  emu->pad_bits[0] = b0;
  
  b1 = emu->pad_bits[1] & 0x0f;
  b1 |= right_RIGHT << 4;
  b1 |= right_LEFT << 5;
  b1 |= right_UP << 6;
  b1 |= right_DOWN << 7;
  emu->pad_bits[1] = b1;
}


