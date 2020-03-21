/* KallistiOS 1.1.8

   panic.c
   (c)2001 Dan Potter
*/

static char id[] = "KOS panic.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp";

/* Setup basic kernel services (printf, etc) */
#include <stdio.h>
#include <malloc.h>

/* If something goes badly wrong in the kernel and you don't think you
   can recover, call this. This is a pretty standard tactic from *nixy
   kernels which ought to be avoided if at all possible. */
void panic(const char *msg) {
	printf("%s\r\n", msg);
	arch_exit();
}

