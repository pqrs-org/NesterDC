/* KallistiOS 1.1.8

   sndmp3.h
   (c)2000 Dan Potter

   sndmp3.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
   
*/

#ifndef __SNDMP3_H
#define __SNDMP3_H

#include <sys/cdefs.h>
__BEGIN_DECLS

int sndmp3_init(const char *fn, int loop);

/* Initialize the MP3 driver; takes an input filename and starts the
   decoding process. */
int sndmp3_start(const char *fn, int loop);

/* Shut everything down */
void sndmp3_shutdown();

__END_DECLS

#endif	/* __SNDMP3_H */

