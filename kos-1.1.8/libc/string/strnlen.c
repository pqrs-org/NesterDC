/* KallistiOS 1.1.8

   strnlen.c
   (c)2000 Dan Potter

   strnlen.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/


#include <string.h>

size_t strnlen(const char * s, size_t count) {
	const char *t = s;

	while (count-- && *t != '\0') t++;
	return t - s;
}
