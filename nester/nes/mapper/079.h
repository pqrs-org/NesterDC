#ifndef __NES_MAPPER_079_H
#define __NES_MAPPER_079_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 79
class NES_mapper79 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper79() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
