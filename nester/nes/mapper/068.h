#ifndef __NES_MAPPER_068_H
#define __NES_MAPPER_068_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 68
class NES_mapper68 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper68() {}

  void  Reset();
  void  MemoryWrite(uint32 addr, uint8 data);

protected:
  void SyncMirror();
  uint8 regs[4];

private:
};
/////////////////////////////////////////////////////////////////////

#endif
