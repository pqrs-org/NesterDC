#ifndef _NES_DEVICE_PADDLE_H_
#define _NES_DEVICE_PADDLE_H_

class NES_device_paddle 
{
public:
  bool initialize();
  
  uint8 ReadReg4016();
  uint8 ReadReg4017(); 
  void WriteReg4016();
  void WriteReg4016_strobe(NES *emu);

  void normalize_position();
  
  enum {
    BUTTON_L = 0x1,
    BUTTON_R = 0x2,
  };
  
  uint8 buttons;
  int x;
  
  uint8 paddle_register;

private:
};

#endif
