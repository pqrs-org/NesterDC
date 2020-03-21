#ifndef _NES_DEVICE_GUN_H_
#define _NES_DEVICE_GUN_H_

/* system depend */
extern uint16 get_nesscreen_pixel_color(int x, int y);

class NES_device_gun 
{
public:
  bool initialize();
  
  uint8 ReadReg4016(); /* for "Space Shadow Gun" */
  uint8 ReadReg4017(NES *emu); 
  void WriteReg4016(NES *emu); /* for "Space Shadow Gun" */
  
  void normalize_position();
  
  enum {
    BUTTON_L = 0x1,
    BUTTON_R = 0x2,
  };
  
  uint8 buttons;
  int x;
  int y;
  
  uint8 spaceshadow_register;
};

#endif
