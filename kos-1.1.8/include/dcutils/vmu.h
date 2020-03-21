/* KallistiOS 1.1.8

   vmu.h
   (c)2002 Dan Potter

   vmu.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/


#ifndef __DCUTILS_VMU_H
#define __DCUTILS_VMU_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/* Sets the icon on all VMUs; expects a 48x32 bitmap */
void vmu_set_icon(const char *vmu_icon);

__END_DECLS

#endif	/* __DCUTILS_VMU_H */

