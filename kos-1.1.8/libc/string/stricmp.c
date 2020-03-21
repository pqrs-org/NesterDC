/* KallistiOS 1.1.8

   stricmp.c
   (c)2000 Dan Potter

   stricmp.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#include <string.h>

/* Works like strcmp, but not case sensitive */
int stricmp(const char * cs,const char * ct) {
	int c1, c2, res;

	while(1) {
		c1 = *cs++; c2 = *ct++;
		if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if ((res = c1 - c2) != 0 || (!*cs && !*ct))
			break;
	}

	return res;
}

/* Also provides strcasecmp (same thing) */
int strcasecmp(const char *cs, const char *ct) {
	return stricmp(cs, ct);
}
