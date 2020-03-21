/* KallistiOS 1.1.8

   dc/fs_vmu.h
   (c)2000-2001 Jordan DeLong

   fs_vmu.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp

*/

#ifndef __DC_FS_VMU_H
#define __DC_FS_VMU_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <kos/fs.h>

/* File functions */
uint32	vmu_open(const char *path, int mode);
void	vmu_close(uint32 hnd);
off_t	vmu_tell(uint32 hnd);
size_t	vmu_total(uint32 hnd);

/* Initialization */
int fs_vmu_init();
int fs_vmu_shutdown();

__END_DECLS

#endif	/* __DC_FS_VMU_H */

