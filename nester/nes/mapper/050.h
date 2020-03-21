#ifndef __NES_MAPPER_050_H
#define __NES_MAPPER_050_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 50
class NES_mapper50 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper50() {}

  void  Reset();
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
