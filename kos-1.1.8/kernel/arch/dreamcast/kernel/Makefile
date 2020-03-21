# KallistiOS 1.1.8
#
# arch/dreamcast/kernel/Makefile
# (c)2000 Dan Potter
#   
# Makefile,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

# Generic kernel abstraction layer: this provides a set of routines
# that the portable part of the kernel expects to find on every
# target processor. Other routines may be present as well, but
# that minimum set must be present.

COPYOBJS = banner.o cache.o dbgio.o entry.o irq.o main.o mm.o panic.o
COPYOBJS += rtc.o ser_console.o syscall.o timer.o
COPYOBJS += init_flags_default.o init_romdisk_default.o
COPYOBJS += mmu.o itlb.o crtbegin.o crtend.o atexit.o
COPYOBJS += exec.o execasm.o
OBJS = $(COPYOBJS) startup.o
SUBDIRS = 

myall: $(OBJS)
	-cp $(COPYOBJS) $(KOS_BASE)/kernel/build/
	-rm banner.c banner.o

include $(KOS_BASE)/Makefile.prefab

banner.o: banner.c

banner.c: make_banner.sh
	./make_banner.sh


