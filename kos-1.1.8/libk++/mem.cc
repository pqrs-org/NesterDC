/* KallistiOS 1.1.8

   libk++/mem.cc

   (c)2002 Gil Megidish
*/

#include <sys/cdefs.h>

CVSID("mem.cc,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp");

/* This file contains definitions for the basic malloc/free operations
   in C++. You can use this tiny replacement for libstdc++ when you
   don't need the whole thing. */

extern "C" void free(void*);
extern "C" void *malloc(unsigned);

void operator delete(void *ptr) {
	if (ptr)
		free(ptr);
}

void* operator new(unsigned len) {
	return malloc(len);
}

void operator delete[](void *ptr) {
	::operator delete(ptr);
}

void* operator new[](unsigned len) {
	return ::operator new(len);
}
                     
