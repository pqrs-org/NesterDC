/* KallistiOS 1.1.8

   usleep.c
   (c)2001 Dan Potter

   usleep.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <arch/timer.h>

/* usleep() */
void usleep(unsigned long usec) {
	timer_spin_sleep (usec / 1000);
}

