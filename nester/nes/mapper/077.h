#ifndef __NES_MAPPER_077_H
#define __NES_MAPPER_077_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 77
class NES_mapper77 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper77() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
