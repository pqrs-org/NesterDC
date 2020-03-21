#ifndef __NES_MAPPER_092_H
#define __NES_MAPPER_092_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 92
class NES_mapper92 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper92() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
