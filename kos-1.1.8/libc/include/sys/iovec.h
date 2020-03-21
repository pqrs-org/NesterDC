/* KallistiOS 1.1.8

   sys/iovec.h
   (c)2001 Dan Potter

   iovec.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

*/

#ifndef __SYS_IOVEC_H
#define __SYS_IOVEC_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stddef.h>

typedef struct iovec {
	char	*iov_base;	/* Base address */
	size_t	iov_len;	/* Length */
} iovec_t;

__END_DECLS

#endif	/* __SYS_IOVEC_H */

