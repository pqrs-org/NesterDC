#include "nes.h"
#include "nes_device_mahjong.h"


/* FIXME! need separate device dependent/independent code */
static uint8 nes_mahjong_table_def[] = { 
  0x14,
  0x1a,
  0x08,
  0x15,
  0x17,
  0x1c,
  0x18,
  0x04,
  0x16,
  0x07,
  0x09,
  0x0a,
  0x0b,
  0x0d,
  0x1d,
  0x1b,
  0x06,
  0x19,
  0x05,
  0x11,
  0x10,
};


bool
NES_device_mahjong::initialize()
{
  int i;
  
  for (i = 0; i < sizeof(nes_mahjong_table_def)/sizeof(nes_mahjong_table_def[0]); ++i)
    nes_mahjong_table[i] = 0;
  
  return true;
}

void 
NES_device_mahjong::setkey(uint8 *matrix)
{
  int i;
  
  for (i = 0; i < sizeof(nes_mahjong_table_def)/sizeof(nes_mahjong_table_def[0]); ++i)
    nes_mahjong_table[i] = matrix[nes_mahjong_table_def[i]];
}


uint8
NES_device_mahjong::ReadReg4017()
{
  uint8 retval = (mahjong_bits & 0x01) << 1;
  mahjong_bits >>= 1;
  
  return retval;
}


void
NES_device_mahjong::WriteReg4016(uint8 data)
{
  mahjong_bits = 0x00;
  
  if((data & 0x06) == 0x02)
  {
     if(nes_mahjong_table[KEY_N]) mahjong_bits |= 0x04;
     if(nes_mahjong_table[KEY_M]) mahjong_bits |= 0x08;
     if(nes_mahjong_table[KEY_L]) mahjong_bits |= 0x10;
     if(nes_mahjong_table[KEY_K]) mahjong_bits |= 0x20;
     if(nes_mahjong_table[KEY_J]) mahjong_bits |= 0x40;
     if(nes_mahjong_table[KEY_I]) mahjong_bits |= 0x80;
   }
  else if((data & 0x06) == 0x04)
  {
     if(nes_mahjong_table[KEY_H]) mahjong_bits |= 0x01;
     if(nes_mahjong_table[KEY_G]) mahjong_bits |= 0x02;
     if(nes_mahjong_table[KEY_F]) mahjong_bits |= 0x04;
     if(nes_mahjong_table[KEY_E]) mahjong_bits |= 0x08;
     if(nes_mahjong_table[KEY_D]) mahjong_bits |= 0x10;
     if(nes_mahjong_table[KEY_C]) mahjong_bits |= 0x20;
     if(nes_mahjong_table[KEY_B]) mahjong_bits |= 0x40;
     if(nes_mahjong_table[KEY_A]) mahjong_bits |= 0x80;
  }
  else if((data & 0x06) == 0x06)
  {
     if(nes_mahjong_table[KEY_RON]) mahjong_bits |= 0x02;
     if(nes_mahjong_table[KEY_REACH]) mahjong_bits |= 0x04;
     if(nes_mahjong_table[KEY_CHII]) mahjong_bits |= 0x08;
     if(nes_mahjong_table[KEY_PON]) mahjong_bits |= 0x10;
     if(nes_mahjong_table[KEY_KAN]) mahjong_bits |= 0x20;
     if(nes_mahjong_table[KEY_START]) mahjong_bits |= 0x40;
     if(nes_mahjong_table[KEY_SELECT]) mahjong_bits |= 0x80;
  }
}

