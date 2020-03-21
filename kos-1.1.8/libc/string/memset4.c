/* KallistiOS 1.1.8

   memset4.c
   (c)2000 Dan Potter

   memset4.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <string.h>

/* This variant was added by Dan Potter for its usefulness in 
   working with Dreamcast external hardware. */
void * memset4(void *s, unsigned long c, size_t count)
{
	unsigned long *xs = (unsigned long *) s;
	count = count / 4;

	while (count--)
		*xs++ = c;

	return s;
}

