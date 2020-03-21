/* KallistiOS 1.1.8

   rtc.c
   (c)2001 Dan Potter
*/

/* Real Time Clock support

   The functions in here return various info about the real-world time and
   date stored in the machine. The general process here is to retrieve
   the date/time value and then use the other functions to interpret it.

   rtc_get_time() should return a UNIX-epoch time stamp, and then the normal
   BSD library functions can be used to interpret that time stamp.

   For the Dreamcast, the RTC is a 32-bit seconds counter located at
   0xa0710000 and 0xa0710004 (each 32-bits long). 0000 contains the high
   16 bits and 0004 contains the low 16 bits. The epoch of this counter is
   January 1, 1950, 00:00. So we just grab that value and add about
   20 years to it.

 */

#include <arch/rtc.h>
#include <dc/g2bus.h>

CVSID("rtc.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Returns the date/time value as a UNIX epoch time stamp */
time_t rtc_unix_secs() {
	uint32 rtcold, rtcnew;
	int i;

	/* Try several times to make sure we don't read one value, then the
	   clock increments itself, then we read the second value. This
	   algorithm is from NetBSD. */
	rtcold = 0;
	for (;;) {
		for (i=0; i<3; i++) {
			rtcnew = ((g2_read_32(0xa0710000) & 0xffff) << 16) | (g2_read_32(0xa0710004) & 0xffff);
			if (rtcnew != rtcold)
				break;
		}
		if (i < 3)
			rtcold = rtcnew;
		else
			break;
	}

	/* Subtract out 20 years */
	rtcnew = rtcnew - 631152000;

	return rtcnew;
}


