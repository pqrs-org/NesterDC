#ifndef __NES_MAPPER_099_H
#define __NES_MAPPER_099_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 99
class NES_mapper99 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper99() {}

  void  Reset();
  void  WriteHighRegs(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

#endif
