#ifndef __DC_SAVING_H
#define __DC_SAVING_H

#include "types.h"
#include "nes.h"

bool sram_save_to_vmu (maple_device_t *dev, NES *emu);
bool sram_load_from_vmu (maple_device_t *dev, NES *emu);

bool disk_save_to_vmu (maple_device_t *dev, NES *emu);
bool disk_load_from_vmu (maple_device_t *dev, NES *emu);

bool snap_save_to_vmu (maple_device_t *dev, NES *emu);
bool snap_load_from_vmu (maple_device_t *dev, NES *emu);

#endif

