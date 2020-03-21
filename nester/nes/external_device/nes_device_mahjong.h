#ifndef _NES_DEVICE_MAHJONG_H_
#define _NES_DEVICE_MAHJONG_H_

class NES_device_mahjong
{
public:
  
  bool initialize();
	
  uint8 ReadReg4017();
  
  void WriteReg4016(uint8 data);
  
  void setkey(uint8 *matrix);
  
  enum {
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_START,
    KEY_SELECT,
    KEY_KAN,
    KEY_PON,
    KEY_CHII,
    KEY_REACH,
    KEY_RON,
  };
  
private:
  uint8 mahjong_bits;
  int nes_mahjong_table[KEY_RON+1];
  
};

#endif


