/* KallistiOS 1.1.8

   _strupr.c
   (c)2002 Brian Peek

   _strupr.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <string.h>
#include <ctype.h>

char *_strupr(char *string)
{
	char *s;

	for(s = string; *s; s++)
		*s = toupper(*s);
	return string;
}

