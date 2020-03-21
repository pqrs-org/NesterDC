#ifndef __NES_MAPPER_046_H
#define __NES_MAPPER_046_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 46
class NES_mapper46 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper46() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  void set_rom_banks();
  uint8 regs[4];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
