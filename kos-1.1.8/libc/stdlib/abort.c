/* KallistiOS 1.1.8

   abort.c
   (c)2001 Dan Potter

   abort.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <arch/arch.h>

CVSID("abort.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* This is probably the closest mapping we've got for abort() */
void abort() {
	arch_exit();
}

