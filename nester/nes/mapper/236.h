#ifndef __NES_MAPPER_236_H
#define __NES_MAPPER_236_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 236
class NES_mapper236 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper236() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  uint8 bank, mode;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
