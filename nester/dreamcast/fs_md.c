/* 2001 Takayama Fumihiko <tekezo@catv296.ne.jp> */

#include <arch/types.h>
#include <kos/limits.h>
#include <kos/fs.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "fs_md.h"


typedef struct {
        int allocate_size;
        int size;
        uint8 *image;
        char filename[32];
} md_file_t;


static struct {
        int is_free;
        int mode;
        int ptr;
        md_file_t *file;
} fh[MAX_MD_FILES];


static size_t filesize_base = 0;
static md_file_t md_image;


file_t md_open(const char *fn, int mode) {
        file_t fd;
        md_file_t *md;
        
        for (fd = 0; fd < MAX_MD_FILES; ++fd) {
                if (fh[fd].is_free)
                        break;
        }
        if (fd == MAX_MD_FILES)
                return 0;
        
        md = &md_image;
        
        if (mode & O_RDONLY) {
                fh[fd].is_free = 0;
                fh[fd].mode = mode;
                fh[fd].ptr = 0;
                fh[fd].file = md;
                
                return fd;
        }
        
        if (mode & O_APPEND) {
                fh[fd].is_free = 0;
                fh[fd].mode = mode;
                fh[fd].ptr = md->size;
                fh[fd].file = md;
                
                return fd;
        }
        
        if (mode & O_WRONLY) {
                fh[fd].is_free = 0;
                fh[fd].mode = mode;
                fh[fd].ptr = 0;
                fh[fd].file = md;
                md->size = 0;
                
                return fd;
        }
        
        return 0;
}


void md_close(file_t fd) {
        if (fd < MAX_MD_FILES)
                fh[fd].is_free = 1;
}


ssize_t md_read(file_t fd, void *buf, size_t bytes) {
        md_file_t *md;
        
        if (fd >= MAX_MD_FILES || fh[fd].is_free)
                return -1;
        
        if (!(fh[fd].mode & O_RDONLY)) 
                return -1;
        
        md = fh[fd].file;
        
        if (fh[fd].ptr + bytes > md->size)
                bytes = md->size - fh[fd].ptr;
        
        memcpy(buf, md->image + fh[fd].ptr, bytes);
        fh[fd].ptr += bytes;
        
        return bytes;
}


ssize_t md_write(file_t fd, const void *buf, size_t bytes) {
        md_file_t *md;
        
        if (fd >= MAX_MD_FILES || fh[fd].is_free)
                return -1;
        
        if (!(fh[fd].mode & O_APPEND) && !(fh[fd].mode & O_WRONLY))
                return -1;
        
        md = fh[fd].file;
        
        if (fh[fd].ptr + bytes > md->allocate_size) {
                /* ... */
#if 0
                md->allocate_size *= 2;
                md->allocate_size += bytes;
                
                md->image = realloc(md->image, md->allocate_size);
#else
                bytes = md->allocate_size - fh[fd].ptr;
#endif
        }
        
        memcpy(md->image + fh[fd].ptr, buf, bytes);
        fh[fd].ptr += bytes;
        if (fh[fd].ptr > md->size)
                md->size = fh[fd].ptr;
        
        return bytes;
}


off_t md_seek(file_t fd, off_t offset, int whence) {
        md_file_t *md;
        
        if (fd >= MAX_MD_FILES || fh[fd].is_free)
                return -1;
        
        md = fh[fd].file;
        
        switch (whence) {
                case SEEK_SET:
                        fh[fd].ptr = offset;
                        break;
                case SEEK_CUR:
                        fh[fd].ptr += offset;
                        break;
                case SEEK_END:
                        fh[fd].ptr = md->size + offset;
                        break;
                default:
                        return -1;
        }
        
	/* Check bounds */
	if (fh[fd].ptr < 0) fh[fd].ptr = 0;
	if (fh[fd].ptr > md->size) fh[fd].ptr = md->size;
	
	return fh[fd].ptr;
}


/* Tell where in the file we are */
off_t md_tell(file_t fd) {
        if (fd>=MAX_MD_FILES || fh[fd].is_free)
                return -1;
        
        return fh[fd].ptr;
}


/* Tell how big the file is */
size_t md_total(file_t fd) {
        if (fd>=MAX_MD_FILES || fh[fd].is_free)
                return -1;
        
        return (fh[fd].file)->size;
}


/* Put everything together */
static vfs_handler vh = {
	{ "/md" },		/* name */
	0, 0, NULL,		/* In-kernel, no cacheing, next */
	md_open,
	md_close,
	md_read,
	md_write,
	md_seek,
	md_tell,
	md_total,
        NULL,
	NULL,
	NULL,
	NULL,
        NULL,
};


/* Initialize the file system */
int fs_md_init(size_t size) {
        int fd;
        
        filesize_base = size;
        
        for (fd = 0; fd < MAX_MD_FILES; ++fd)
                fh[fd].is_free = 1;
        fh[0].is_free = 0;
        
#if 1
        /* test coding */
        md_image.allocate_size = size;
        md_image.size = 0;
        md_image.image = malloc(size);
        *(md_image.filename) = '\0';
#endif
        
	/* Register with VFS */
	return fs_handler_add("/md", &vh);
}

/* De-init the file system */
int fs_md_shutdown() {
        free(md_image.image);
        
	return fs_handler_remove(&vh);
}

