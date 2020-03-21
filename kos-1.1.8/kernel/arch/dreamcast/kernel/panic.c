/* KallistiOS 1.1.8

   panic.c
   (c)2001 Dan Potter
*/

#include <stdio.h>
#include <malloc.h>
#include <arch/arch.h>

CVSID("panic.c,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp");

/* If something goes badly wrong in the kernel and you don't think you
   can recover, call this. This is a pretty standard tactic from *nixy
   kernels which ought to be avoided if at all possible. */
void panic(const char *msg) {
	printf("kernel panic: %s\r\n", msg);
#if 1
        vid_set_start(0);
        bfont_draw_str(vram_s + 640 * 24, 640, 1, msg);
        timer_spin_sleep(2000);
#endif
	arch_abort();
}

