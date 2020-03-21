#ifndef __NES_MAPPER_016_H
#define __NES_MAPPER_016_H

#include "nes_mapper.h"

/////////////////////////////////////////////////////////////////////
// Mapper 16
class NES_mapper16 : public NES_mapper
{
  friend class NES_SNSS;

public:
  ~NES_mapper16() {}

  void  Reset();
  void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
  void  MemoryWrite(uint32 addr, uint8 data);
  void  MemoryReadSaveRAM(uint32 addr);
  void  HSync(uint32 scanline);
  void  SetBarcodeValue(uint32 value_low, uint32 value_high);

protected:
  void  MemoryWrite2(uint32 addr, uint8 data);
  void  MemoryWrite3(uint32 addr, uint8 data);

  uint8 patch;
  uint8 regs[3];

  uint8 serial_out[0x2000];

  uint8 eeprom_cmd[4];
  uint8 eeprom_status;
  uint8 eeprom_mode;
  uint8 eeprom_pset;
  uint8 eeprom_addr, eeprom_data;
  uint8 eeprom_wbit, eeprom_rbit;

  uint8 barcode[256];
  uint8 barcode_status;
  uint8 barcode_pt;
  uint8 barcode_pt_max;
  uint8 barcode_phase;
  uint32 barcode_wait;
  uint8 barcode_enabled;

  uint8 irq_enabled;
  uint32 irq_counter;
  uint32 irq_latch;

private:
};
/////////////////////////////////////////////////////////////////////

#endif
