/* KallistiOS 0.80

   os/abi/serial.h
   (c)2001 Dan Potter

   serial.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/

#ifndef __OS_ABI_SERIAL_H
#define __OS_ABI_SERIAL_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <os/process.h>
#include <kos/dbgio.h>

typedef struct {
	service_abi_t	hdr;		/* Header info */

	void		(*write)(uint8 *data, int len);
	void		(*read)(uint8 *data, int len);
	void		(*flush)();
} abi_serial_t;

__END_DECLS

#endif	/* __OS_ABI_SERIAL_H */

