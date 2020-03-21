#ifndef __NES_MAPPER_225_H
#define __NES_MAPPER_225_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 225
class NES_mapper225 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper225() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
