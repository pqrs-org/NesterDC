#ifndef _NES_DEVICE_HYPERSHOT_H_
#define _NES_DEVICE_HYPERSHOT_H_

class NES_device_hypershot
{
public:
  bool initialize();
  
  uint8 ReadReg4017(); 
  void WriteReg4016(NES *emu);
  
private:
  uint8 hs_register;
};

#endif
