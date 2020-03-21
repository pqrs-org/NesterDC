#ifndef __NES_MAPPER_005_H
#define __NES_MAPPER_005_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 5
class NES_mapper5 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper5() {}

  void  Reset();

  uint8 MemoryReadLow(uint32 addr);
  void  MemoryWriteLow(uint32 addr, uint8 data);
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  HSync(uint32 scanline);
  uint8 PPU_Latch_RenderScreen(uint8 mode, uint32 addr);

protected:
  void MMC5_set_CPU_bank(uint8 page, uint8 bank);
  void MMC5_set_WRAM_bank(uint8 page, uint8 bank);
  void sync_Chr_banks(uint8 page);

  uint32 wb[8];
  uint8 wram[8*0x2000];
  uint8 wram_size;

  uint8 chr_reg[8][2];

  uint8 irq_enabled;
  uint8 irq_status;
  uint32 irq_line;

  uint32 value0;
  uint32 value1;

  uint8 wram_protect0;
  uint8 wram_protect1;
  uint8 prg_size;
  uint8 chr_size;
  uint8 gfx_mode;
  uint8 split_control;
  uint8 split_bank;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
