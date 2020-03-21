#ifndef __NES_MAPPER_181_H
#define __NES_MAPPER_181_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 181
class NES_mapper181 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper181() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
