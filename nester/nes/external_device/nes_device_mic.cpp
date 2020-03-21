#include "nes.h"
#include "nes_device_mic.h"

bool
NES_device_mic::initialize()
{
  mic_bits = 0;
  
  return true;
}

uint8 
NES_device_mic::ReadReg4016()
{
  return mic_bits;
}


void
NES_device_mic::WriteReg4016(NES *emu)
{
  /* mic input binds on NES pad #2's START */
  mic_bits = (emu->pad_bits[1] & 0x08) >> 1;
}


