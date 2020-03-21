/* KallistiOS 1.1.8

   unistd.h
   (c)2000-2001 Dan Potter

   unistd.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

*/

#ifndef __UNISTD_H
#define __UNISTD_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stddef.h>
#include <arch/types.h>

#define true (1)
#define false (0)

void usleep(unsigned long usec);

__END_DECLS

#endif	/* __UNISTD_H */

