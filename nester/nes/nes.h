/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

/* 
** NesterDC by Ken Friece
** Any changes that I made to the original source are 
** marked with in the following way:
** KF DD/MM/YYYY <description of the change> 
*/

#ifndef _NES_H_
#define _NES_H_

#include <stdio.h>

#include "types.h"
#include "nes_6502.h"
#include "mapper/nes_mapper_decl.h"
#include "nes_rom.h"
#include "nes_ppu.h"
#include "nes_apu_wrapper.h"
#include "nes_pad.h"
#include "external_device/nes_device_cclimber.h"
#include "external_device/nes_device_gun.h"
#include "external_device/nes_device_kbd.h"
#include "external_device/nes_device_hypershot.h"
#include "external_device/nes_device_mic.h"
#include "external_device/nes_device_paddle.h"
#include "external_device/nes_device_mahjong.h"
#include "nnnesterj_cheat.h"

class NES_SNSS;

class NES
{
  // friend classes
  friend class NES_6502;
  friend class NES_PPU;
  friend class NES_APU;
  friend class NES_SNSS;
  friend class NES_mapper;
  friend class NES_device_cclimber;
  friend class NES_device_gun;
  friend class NES_device_hypershot;
  friend class NES_device_mic;
  friend class NES_device_paddle;
  friend class NES_device_mahjong;
#include "mapper/nes_mapper_friend_decl.h"
  
public:
  NES(sound_mgr* _sound_mgr);
  bool initialize (const char *filename);
  ~NES();
  
  void new_snd_mgr(sound_mgr* _sound_mgr);
  
  NES_pad* get_pad(int num) { return pad + num; }
  NES_device_cclimber NES_cclimber;
  NES_device_gun NES_gun;
  NES_device_kbd NES_kbd;
  NES_device_hypershot NES_hypershot;
  NES_device_mic mic;
  NES_device_paddle NES_paddle;
  NES_device_mahjong NES_mahjong;
  
  void emulate_frame (uint16 *screen);
  void emulate_frame_skip (); 
  
  void reset();
  void softreset();
  
  void set_exsound_enable(bool exsound_enable) { apu->set_exsound_enable(exsound_enable); }
  
  uint8 get_mapper_num() { return ROM->get_mapper_num (); }
  const char* getROMname();
  const char* getROMpath();
  const char* getROMcrc() {
    return ROM->GetRomCrc(); 
  }
  NES_ROM::rom_monitor_type get_monitor_type () { return ROM->get_monitor_type (); }
  
  bool loadState(const char* fn);
  bool saveState(const char* fn);
  
  void freeze();
  void thaw();
  boolean frozen();
  
#ifndef __DREAMCAST__
  /* for draw_pattern_table */
  uint8 *ppu_vram_banks(int bank) { return ppu->PPU_VRAM_banks[bank]; } 
  uint16 *ppu_bg_pal_color() { return ppu->bg_pal_color; }
#endif
  
  enum {
    NES_NUM_VBLANK_LINES = 20,
    NES_NUM_FRAME_LINES = 240,

    // these are 0-based, and actions occur at start of line
    NES_NMI_LINE = 241,
    NES_VBLANK_FLAG_SET_LINE = 241,
    NES_VBLANK_FLAG_RESET_LINE = 261,
    NES_SPRITE0_FLAG_RESET_LINE = 261,

    NES_COLOR_BASE = 0x40, // NES palette is set starting at color 0x40 (64)
    NES_NUM_COLORS = 64    // 64 colors in the NES palette
  };
  
  uint32 crc32()  { return ROM->crc32();  }
  uint32 fds_id() { return ROM->fds_id(); }
  NES_ROM::rom_image_type get_rom_image_type () { return ROM->get_rom_image_type (); }
  
  float CYCLES_PER_LINE;
  float CYCLES_BEFORE_NMI;
  boolean BANKSWITCH_PER_TILE;
  boolean DPCM_IRQ;
  
  // Disk System
  uint8 GetDiskData(uint32 pt);
  uint8 GetDiskSideNum();
  uint8 GetDiskSide();
  void SetDiskSide(uint8 side);
  uint8 DiskAccessed();
  char disksys_rom_filename[256];
  
  /* External Device */
  typedef enum {
    EX_NONE,
    EX_FAMILY_KEYBOARD,
    EX_OPTICAL_GUN,
    EX_SPACE_SHADOW_GUN,
    EX_HYPER_SHOT,
    EX_CRAZY_CLIMBER,
    EX_ARKANOID_PADDLE,
    EX_MAHJONG,
    EX_LAST_ITEM,
  } nes_external_device_type_t;
  
  nes_external_device_type_t ex_controller_type;
  const char *get_external_device_name() {
    const char *names[] = {
      "NONE",
      "Family Keyboard",
      "Optical Gun",
      "Space Shadow Gun",
      "Hyper Shot",
      "Crazy Climber", 
      "Arkanoid Paddle", 
      "Mahjong", 
      "",
    };
    return names[ex_controller_type];
  }
  
  void change_external_device() {
    ex_controller_type = nes_external_device_type_t(ex_controller_type + 1);
    if (ex_controller_type == EX_LAST_ITEM)
      ex_controller_type = EX_NONE;
  }
  
  // Game Genie
  void clear_genie () { genie_num = 0; }
  void load_genie (const char *genie);
  void set_genie ();
  uint8 GetGenieCodeNum() { return genie_num; }
  uint32 GetGenieCode(uint8 num) { return genie_code[num]; }
  
  /* NNNesterJ cheat code */
  static const int NES_NNNESTERJ_CHEATINFO_SIZE = 100;
  nes_nnnesterj_cheatinfo_t nes_nnnesterj_cheatinfo[NES_NNNESTERJ_CHEATINFO_SIZE];
  int nes_nnnesterj_cheatinfo_size;
  int nes_nnnesterj_cheatinfo_alive_num;
  
  /* NNNesterJ cheat code */
  void load_nnnesterj_cheat (const char *filename);
  void set_nnnesterj_cheat_alive_num ();
  void apply_nnnesterj_cheat();
  
  // frame-IRQ
  uint8 frame_irq_enabled;
  uint8 frame_irq_disenabled;
  
  // these are called by the CPU
  uint8 MemoryRead(uint32 addr);
  void  MemoryWrite(uint32 addr, uint8 data);
  
  // SaveRAM control
  void  WriteSaveRAM(uint32 addr, uint8 data) { SaveRAM[addr] = data;}
  uint8 ReadSaveRAM(uint32 addr) { return SaveRAM[addr]; }
  
  // file stuff
  bool has_sram () { return ROM->has_save_RAM (); }
  const uint8 *get_SaveRAM () { return SaveRAM; }
  void set_SaveRAM(const uint8 *buffer, int size);
  bool is_invalid_sram ();
  
  /* change 0x2000 to 0x10000 for mapper 005 */
  static const int sram_buflen = 0x10000; 
  static const int fds_save_buflen = 16 + (4 * 65500) * 2;
  /* snss_buflen: 
     0x8000: fds's WRAM
     0x40000 * 2: fds's quick disk state */
  static const int snss_buflen = sizeof(SNSS_FILE) + 0x8000 + 0x40000 * 2;
  /* buf length must be fds_save_buflen */
  void make_savedata(uint8 *buf);
  void restore_savedata(uint8 *buf);
  
protected:
  sound_mgr* snd_mgr;
  NES_6502* cpu;
  NES_PPU* ppu;
  NES_APU* apu;
  NES_ROM* ROM;
  NES_mapper* mapper;
  
  boolean is_frozen;
  
  float  ideal_cycle_count;   // number of cycles that should have executed so far
  uint32 emulated_cycle_count;  // number of cycles that have executed so far
  
  // internal memory
  uint8 RAM[0x800];
  uint8 SaveRAM[sram_buflen];
  
  // joypad stuff
  NES_pad pad[4];
  boolean pad_strobe;
  uint32 pad_bits[4];
  
  // Disk System
  uint8 disk_side_flag;
  
  bool loadROM(const char* fn); 
  void freeROM();
  
  // Game Genie
  uint8 genie_num;
  uint32 genie_code[256];
  
  // internal read/write functions
  uint8 ReadRAM(uint32 addr);
  void  WriteRAM(uint32 addr, uint8 data);
  
  uint8 ReadLowRegs(uint32 addr);
  void  WriteLowRegs(uint32 addr, uint8 data);
  
  uint8 ReadHighRegs(uint32 addr);
  void  WriteHighRegs(uint32 addr, uint8 data);
  
  void  emulate_CPU_cycles(float num_cycles);
  void  trim_cycle_counts();
  
private:
  NES_SNSS* snss;
  
  /* helper for emulate_frame */
  void emulate_frame_skip_line_generic (int i);
  void emulate_frame_skip_line_mmc5 (int i);
  void emulate_frame_skip_line_mmc2 (int i);
  void emulate_frame_skip_line_bankswitch_per_tile (int i);
  
  void emulate_frame_line_generic (int i, uint16 *cur_line);
  void emulate_frame_line_mmc5 (int i, uint16 *cur_line);
  void emulate_frame_line_mmc2 (int i, uint16 *cur_line);
  void emulate_frame_line_bankswitch_per_tile (int i, uint16 *cur_line);
};

#endif

