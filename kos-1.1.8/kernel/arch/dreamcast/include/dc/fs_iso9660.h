/* KallistiOS 1.1.8

   dc/fs/iso9660.h
   (c)2000-2001 Dan Potter

   fs_iso9660.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/

#ifndef __DC_FS_ISO9660_H
#define __DC_FS_ISO9660_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>
#include <kos/limits.h>
#include <kos/fs.h>

#define MAX_ISO_FILES 8

/* Prototypes */
uint32	iso_open(const char *path, int flags);
void	iso_close(uint32 fd);
ssize_t	iso_read(uint32 fd, void *buf, size_t count);
off_t	iso_seek(uint32 fd, off_t offset, int whence);
off_t	iso_tell(uint32 fd);
size_t	iso_total(uint32 fd);
int	iso_ioctl(uint32 fd, void *data, size_t size);

dirent_t *iso_readdir(uint32 fd);

int fs_iso9660_init();
int fs_iso9660_shutdown();

__END_DECLS

#endif	/* __DC_FS_ISO9660_H */

