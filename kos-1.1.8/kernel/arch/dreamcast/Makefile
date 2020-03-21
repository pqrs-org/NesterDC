# KallistiOS 1.1.8
#
# arch/dreamcast Makefile
# (c)2001 Dan Potter
#   
# Makefile,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
	
SUBDIRS=fs hardware kernel math sound util

ifeq ($(KOS_SUBARCH), navi)
	SUBDIRS += navi
endif

include $(KOS_BASE)/Makefile.rules

all: subdirs
clean: clean_subdirs
