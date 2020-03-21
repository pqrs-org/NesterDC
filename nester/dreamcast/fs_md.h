#ifndef __KOS_FS_MD_H
#define __KOS_FS_MD_H

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif


#include <arch/types.h>
#include <kos/limits.h>
#include <kos/fs.h>

#define MAX_MD_FILES 8

file_t md_open(const char *fn, int mode);
void md_close(file_t fd);
ssize_t md_read(file_t fd, void *buf, size_t bytes);
ssize_t md_write(file_t fd, const void *buf, size_t bytes);
off_t md_tell(file_t fd);
size_t md_total(file_t fd);

int fs_md_init(size_t size);
int fs_md_shutdown();

#ifdef __cplusplus
}
#endif

#endif


