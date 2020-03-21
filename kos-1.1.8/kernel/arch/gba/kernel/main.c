/* KallistiOS 1.1.8

   main.c
   (c)2001 Dan Potter
*/

static char id[] = "KOS main.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp";

#include <string.h>

/* This is the entry point inside the C program */
int arch_main() {
	if (mm_init() < 0)
		return 0;

	return main(0, NULL);
}

/* Called to shut down the system */
void arch_exit() {
}

/* Called from syscall to reboot the system */
void arch_reboot() {
	/* Reboot */
	{ void (*rb)() = (void (*)())0x00000000; rb(); }
}


/* When you make a function called main() in a GCC program, it wants
   this stuff too. */
void _main() { }
void atexit() { }

/* GCC 3.0 also wants these */
void __gccmain() { }

