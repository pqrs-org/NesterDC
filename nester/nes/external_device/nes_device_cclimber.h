#ifndef _NES_DEVICE_CCLIMBER_H_
#define _NES_DEVICE_CCLIMBER_H_

class NES_device_cclimber
{
public:
  bool initialize();
  
  uint8 ReadReg4016();	
  void WriteReg4016(NES *emu);
  
  bool left_UP;
  bool left_DOWN;
  bool left_LEFT;
  bool left_RIGHT;
  
  bool right_UP;
  bool right_DOWN;
  bool right_LEFT;
  bool right_RIGHT;
};

#endif
