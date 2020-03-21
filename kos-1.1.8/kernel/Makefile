# KallistiOS 1.1.8
#
# kernel Makefile
# (c)2001 Dan Potter
#   
# Makefile,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

OBJS =
SUBDIRS = arch fs misc mm thread net

all: subdirs
	rm -f $(KOS_BASE)/lib/libkallisti.a
	$(KOS_AR) rcs $(KOS_BASE)/lib/libkallisti.a build/*.o build/libc/*.o

include $(KOS_BASE)/Makefile.prefab

clean: clean_subdirs
	rm -f build/*.o
	rm -f build/libc/*.o

run:


