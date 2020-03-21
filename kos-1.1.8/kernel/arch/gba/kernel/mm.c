/* KallistiOS 1.1.8

   mm.c
   (c)2000-2001 Dan Potter
*/

static char id[] = "KOS mm.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp";


/* Defines a simple UNIX-style memory pool system. */

#include <arch/types.h>

/* The end of the program is always marked by the '_end' symbol. So we'll
   longword-align that and add a little for safety. sbrk() calls will
   move up from there. */
extern unsigned long end;
static void *sbrk_base;

/* MM-wide initialization */
int mm_init() {
	int base = (int)(&end);
	base = (base/4)*4 + 4;		/* longword align */
	sbrk_base = (void*)base;
	
	return 0;
}

/* Simple sbrk function */
void* sbrk(unsigned long increment) {
	void *base = sbrk_base;

	if (increment & 3)
		increment = (increment + 4) & ~3;
	sbrk_base += increment;

	if ( ((uint32)sbrk_base) >= (0x02040000 - 4096) ) {
		panic("out of memory; about to run over kernel stack");
	}
	
	return base;
}


