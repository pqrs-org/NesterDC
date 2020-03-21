/* KallistiOS 1.1.8

   memcpy4.c
   (c)2000 Dan Potter

   memcpy4.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <string.h>

/* This variant was added by Dan Potter for its usefulness in 
   working with Dreamcast external hardware. */
void * memcpy4(void *dest, const void *src, size_t count)
{
	unsigned long *tmp = (unsigned long *) dest;
	unsigned long *s = (unsigned long *) src;
	count = count/4;

	while (count--)
		*tmp++ = *s++;

	return dest;
}
