/* KallistiOS 1.1.8

   gettimeofday.c
   (c)2002 Dan Potter

   gettimeofday.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <assert.h>
#include <time.h>
#include <arch/timer.h>
#include <arch/rtc.h>

/* This is kind of approximate and works only with "localtime" */
int gettimeofday(struct timeval *tv, struct timezone *tz) {
	uint32	m, s;
	
	assert( tv != NULL );

	timer_ms_gettime(&s, &m);
	tv->tv_sec = rtc_unix_secs() + s;
	tv->tv_usec = m * 1000;

	return 0;
}
