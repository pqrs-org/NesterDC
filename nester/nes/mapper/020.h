#ifndef __NES_MAPPER_020_H
#define __NES_MAPPER_020_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 20
class NES_mapper20 : public NES_mapper
{
  friend class NES_SNSS;
  
public:
  NES_mapper20() { is_load_fds = false; }
  ~NES_mapper20() {}
  bool initialize (NES *parent, NES_PPU *ppu);

  void  Reset();
  uint8 MemoryReadLow(uint32 addr);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);
  void  VSync();

  uint8 GetDiskSideNum()                   { return ROM_banks[4]; }
  uint8 GetDiskSide()                      { return current_side; }
  void  SetDiskSide(uint8 side)            { if(side <= ROM_banks[4]) {current_side = side; insert_wait = 0;} }
  uint8 GetDiskData(uint32 pt)             { return disk[pt]; }
  void  SetDiskData(uint32 pt, uint8 data) { disk[pt] = data; }
  uint8 DiskAccessed()                     { uint8 retval = access_flag; access_flag = 0; return retval; }

protected:
  uint8 bios[0x2000];
  uint8 wram[0x8000];
  uint8 disk[0x40000];

  uint8 irq_enabled;
  uint32 irq_counter;
  uint32 irq_latch;
  uint8 irq_wait;
  uint32 irq_counter_amount; 

  uint8 disk_enabled;
  uint32 head_position;
  uint8 write_skip;
  uint8 disk_status;
  uint8 write_reg;
  uint8 current_side;

  uint8 access_flag;
  uint8 last_side;
  uint8 insert_wait;

  void SNSS_fixup();

private:
  bool is_load_fds;
};
/////////////////////////////////////////////////////////////////////

#endif
