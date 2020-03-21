/* KallistiOS 1.1.8

   time.c
   (c)2002 Dan Potter

   time.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <time.h>
#include <arch/rtc.h>

/* time() */
time_t time(time_t *ptr) {
	time_t t;

	t = rtc_unix_secs();
	if (ptr) {
		*ptr = t;
	}
	return t;
}

/* time() */
double difftime(time_t time1, time_t time0) {
	return (double)(time1 - time0);
}

