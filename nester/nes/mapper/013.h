#ifndef __NES_MAPPER_013_H
#define __NES_MAPPER_013_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 13
class NES_mapper13 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper13() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 prg_bank;
  uint8 chr_bank;

  void SNSS_fixup(); // HACK HACK HACK HACK

private:
};
/////////////////////////////////////////////////////////////////////

#endif
