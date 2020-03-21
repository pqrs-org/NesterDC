#ifndef __SDL_SAVING_H
#define __SDL_SAVING_H

#include "types.h"
#include "nes.h"

bool sram_save (NES *emu);
bool sram_load (NES *emu);

bool disk_save (NES *emu);
bool disk_load (NES *emu);

#endif

