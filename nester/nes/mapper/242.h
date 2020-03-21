#ifndef __NES_MAPPER_242_H
#define __NES_MAPPER_242_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 242
class NES_mapper242 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper242() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
