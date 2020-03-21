#include "nes.h"
#include "nes_device_paddle.h"


bool 
NES_device_paddle::initialize()
{
  buttons = 0;
  x = 128;
  
  paddle_register = 0;
  
  return true;
}


uint8 
NES_device_paddle::ReadReg4016()
{
  return (buttons & BUTTON_L) << 1;
}


uint8
NES_device_paddle::ReadReg4017()
{
  uint8 retval = (paddle_register & 0x01) << 1;
  paddle_register >>= 1;
  
  return retval;
}


void
NES_device_paddle::WriteReg4016_strobe(NES *emu)
{
  emu->pad_bits[0] |= ((buttons & BUTTON_R) << 1);
}


void
NES_device_paddle::WriteReg4016()
{
  paddle_register = 0;
  
  if (x < 32) x = 32;
  if (x > 176) x = 176;
  uint8 px = 0xff - (uint8)(0x52+172*(x - 32)/144);
  
  paddle_register |= (px & 0x01) << 7;
  paddle_register |= (px & 0x02) << 5;
  paddle_register |= (px & 0x04) << 3;
  paddle_register |= (px & 0x08) << 1;
  paddle_register |= (px & 0x10) >> 1;
  paddle_register |= (px & 0x20) >> 3;
  paddle_register |= (px & 0x40) >> 5;
  paddle_register |= (px & 0x80) >> 7;
}


void
NES_device_paddle::normalize_position()
{
  if (x < 0) x = 0;
  if (x > 255) x = 255;
}
