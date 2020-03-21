#ifndef __NES_MAPPER_185_H
#define __NES_MAPPER_185_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 185
class NES_mapper185 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper185() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 patch;
  uint8 dummy_chr_rom[0x400];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
