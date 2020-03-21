/* KallistiOS 1.1.8

   sfxmgr.h
   (c)2000 Dan Potter

   sfxmgr.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
   
*/

#ifndef __SFXMGR_H
#define __SFXMGR_H

#include <sys/cdefs.h>
__BEGIN_DECLS

int sfx_load(const char *fn);
void sfx_play(int idx, int vol, int pan);
void sfx_unload_all();

__END_DECLS

#endif	/* __SFXMGR_H */

