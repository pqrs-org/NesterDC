#ifndef __NES_MAPPER_228_H
#define __NES_MAPPER_228_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 228
class NES_mapper228 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper228() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
