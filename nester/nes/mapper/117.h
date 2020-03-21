#ifndef __NES_MAPPER_117_H
#define __NES_MAPPER_117_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 117
class NES_mapper117 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper117() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_line;
  uint8 irq_enabled1;
  uint8 irq_enabled2;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
