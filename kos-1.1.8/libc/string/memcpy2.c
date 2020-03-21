/* KallistiOS 1.1.8

   memcpy2.c
   (c)2001 Dan Potter

   memcpy2.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <string.h>

/* This variant was added by Dan Potter for its usefulness in 
   working with GBA external hardware. */
void * memcpy2(void *dest, const void *src, size_t count)
{
	unsigned short *tmp = (unsigned short *) dest;
	unsigned short *s = (unsigned short *) src;
	count = count/2;

	while (count--)
		*tmp++ = *s++;

	return dest;
}
