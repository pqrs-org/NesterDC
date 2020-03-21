# KallistiOS 1.1.8
#
# arch/dreamcast/hardware/maple/Makefile
# (c)2000-2001 Dan Potter
#
# Makefile,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

# Core maple handling stuff
OBJS := maple_driver.o maple_enum.o maple_globals.o
OBJS := $(OBJS) maple_init_shutdown.o maple_irq.o
OBJS := $(OBJS) maple_queue.o maple_utils.o

# Various input devices
OBJS := $(OBJS) controller.o keyboard.o mouse.o

# Output devices
OBJS := $(OBJS) vmu.o

# Old maple compatibility
OBJS := $(OBJS) maple_compat.o

SUBDIRS =

include $(KOS_BASE)/Makefile.prefab



