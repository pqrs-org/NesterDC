OBJS = nes.o nes_mapper.o nes_pad.o nes_rom.o snss.o nnnesterj_cheat.o
SUBDIRS = apu cpu external_device libsnss ppu mapper 

include $(NESTER_BASE)/Makefile.prefab

nes.o: nes.cpp nes.h nes_set_cycles.cpp nes_set_controller.cpp nes_emulate_frame.cpp nes_pad.h
nes_rom.o: nes_rom.cpp nes_rom_correct.cpp

