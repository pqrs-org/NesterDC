#ifndef __NES_MAPPER_078_H
#define __NES_MAPPER_078_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 78
class NES_mapper78 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper78() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
