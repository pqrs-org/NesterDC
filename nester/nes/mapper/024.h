#ifndef __NES_MAPPER_024_H
#define __NES_MAPPER_024_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 24
class NES_mapper24 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper24() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);

protected:
  uint8 irq_enabled_l; /* irq_enabled & 0x1 */
  uint8 irq_enabled_h; /* irq_enabled & 0x2 */
  uint8 irq_counter;
  uint8 irq_latch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
