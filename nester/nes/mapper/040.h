#ifndef __NES_MAPPER_040_H
#define __NES_MAPPER_040_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 40 (smb2j)
class NES_mapper40 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper40() {}

  void  Reset();

  void  MemoryWrite(uint32 addr, uint8 data);

  void  HSync(uint32 scanline);
protected:
  uint8 irq_enabled;
  uint32 lines_to_irq;
private:
};
/////////////////////////////////////////////////////////////////////

#endif
