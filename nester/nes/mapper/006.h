#ifndef __NES_MAPPER_006_H
#define __NES_MAPPER_006_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 6
class NES_mapper6 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper6() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;
  uint32 irq_counter;
  uint8 chr_ram[4*0x2000];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
