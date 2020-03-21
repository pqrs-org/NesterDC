/* KallistiOS 1.1.8

   clock.c
   (c)2002 Jeffrey McBeth

   clock.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <time.h>

/* clock() */
/* According to the POSIX standard, if the process time can't be determined
 * clock should return (clock_t)-1
 */
clock_t clock(void) {
	return (clock_t)-1;
}

