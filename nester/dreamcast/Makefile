OBJS = dc_utils.o dc_sound_mgr.o dc_main.o 
OBJS += fs_md.o dc_vmu.o dc_saving.o dc_sound.o dc_gg.o
SUBDIRS = menu_common menu_nesterdc

include $(NESTER_BASE)/Makefile.prefab

dc_sound.o: sound/aica_fw.h
dc_utils.o: lcd_icons/3x5fonts.h
dc_main.o: testing.cpp
dc_saving.o: icons/nesterdc-icon.h
