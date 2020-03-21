/* KallistiOS 1.1.8

   exit.c
   (c)2001 Dan Potter

   exit.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <arch/arch.h>

/* exit() */
void exit() {
	arch_exit();
}

