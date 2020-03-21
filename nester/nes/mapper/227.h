#ifndef __NES_MAPPER_227_H
#define __NES_MAPPER_227_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 227
class NES_mapper227 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper227() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
