#ifndef __NES_MAPPER_113_H
#define __NES_MAPPER_113_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 113
class NES_mapper113 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper113() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
