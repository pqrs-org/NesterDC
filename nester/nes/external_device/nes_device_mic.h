#ifndef _NES_DEVICE_MIC_H_
#define _NES_DEVICE_MIC_H_

class NES_device_mic
{
public:
  bool initialize();
  
  uint8 ReadReg4016();	
  void WriteReg4016(NES *emu);

private:
  uint8 mic_bits;
  
};

#endif
