# KallistiOS 1.1.8
#
# Root Makefile
# (c)2000-2001 Dan Potter
#   
# Makefile,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

all: build-utils build-libc build-libm build-libk++ build-kernel 

clean:
	-rm -f lib/*
	$(KOS_MAKE) -C utils clean
	$(KOS_MAKE) -C libc clean
	$(KOS_MAKE) -C libm clean
	$(KOS_MAKE) -C libk++ clean
	$(KOS_MAKE) -C kernel clean

build-utils:
	$(KOS_MAKE) -C utils

build-libc:
	$(KOS_MAKE) -C libc

build-libm:
	$(KOS_MAKE) -C libm

build-libk++:
ifdef KOS_CCPLUS
	$(KOS_MAKE) -C libk++
endif

build-kernel:
	$(KOS_MAKE) -C kernel

build-addons:
	$(KOS_MAKE) -C addons

build-examples:
	$(KOS_MAKE) -C examples


