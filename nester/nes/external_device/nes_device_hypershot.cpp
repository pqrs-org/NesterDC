#include "nes.h"
#include "nes_device_hypershot.h"


bool
NES_device_hypershot::initialize()
{
  hs_register = 0;
  
  return true;
}


uint8 
NES_device_hypershot::ReadReg4017()
{
  return hs_register;
}


void
NES_device_hypershot::WriteReg4016(NES *emu)
{
  hs_register = 0x00;
  
  if (emu->pad_bits[0] & 0x01) hs_register |= 0x02;
  if (emu->pad_bits[0] & 0x02) hs_register |= 0x04;
  if (emu->pad_bits[1] & 0x01) hs_register |= 0x08;
  if (emu->pad_bits[1] & 0x02) hs_register |= 0x10;
}


