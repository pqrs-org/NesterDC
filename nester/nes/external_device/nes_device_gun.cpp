#include "nes.h"
#include "nes_device_gun.h"


bool 
NES_device_gun::initialize()
{
  buttons = 0;
  x = 255/2;
  y = 239/2;
  
  spaceshadow_register = 0;
  
  return true;
}

/* for "Space Shadow Gun" */
uint8 
NES_device_gun::ReadReg4016()
{
  uint8 retval;
  
  retval = (spaceshadow_register & 0x01) << 1;
  spaceshadow_register >>= 1;
  
  return retval;
}


uint8
NES_device_gun::ReadReg4017(NES *emu)
{
  uint8 retval = 0x00;
  uint16 c;
  
  if (emu->ex_controller_type == NES::EX_SPACE_SHADOW_GUN)
  {
    if (buttons & BUTTON_L) retval |= 0x10;
  }
  else
  {
    if (buttons) retval |= 0x10;
  }
  
  c = get_nesscreen_pixel_color (x, y);
  
  if (emu->crc32() == 0xbc1dce96) // Chiller (#11)
  {
    //nerd hack	    
    if (c != 16403 && c != 34830 && c != 43010 && c != 30784 && c != 32798 && c != 47127 && c != 57355)
      retval |= 0x08;
  }
  else
  {
    retval |= ((c != 65535) << 3);
  }
  
  return retval;
}


/* for "Space Shadow Gun" */
void
NES_device_gun::WriteReg4016(NES *emu)
{
  spaceshadow_register = (emu->pad_bits[0] & 0xFC);
  if (buttons & BUTTON_R) spaceshadow_register |= 0x02;
}


void
NES_device_gun::normalize_position()
{
  if (y < 0) y = 0;
  if (y > 239) y = 239;
  if (x < 0) x = 0;
  if (x > 255) x = 255;
}
