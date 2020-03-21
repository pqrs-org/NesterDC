/* KallistiOS 1.1.8

   fs_builtin.c
   (c)2000-2001 Dan Potter

*/

/*

This is a simple VFS handler that implements an in-kernel file system. All
of the files to be included in this file system are stored statically in
the kernel binary itself. In this way you can make a composite binary that
you can upload in one shot, or you can also use it to bootstrap the 
kernel binary.

In order for this to work, it needs a file system table somewhere. This
is not included in the kernel library itself, but you are expected to
provide one when you link it. The table may be NULL in which case there
is no built-in file system (initialization will not occur). Call
fs_builtin_set_table() to setup a table.

The functions are generally considered private but if you want to access
them for some reason, they are all public.

*/

#include <string.h>
#include <kos/fs_builtin.h>
#include <kos/thread.h>
#include <arch/spinlock.h>

CVSID("fs_builtin.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Static file table -- this should be set exactly once by the main()
   of the linking program. It can only be called once because changing
   tables mid-stream would break open files. */
static fs_builtin_ent *tbl;
static int tblcnt;
int fs_builtin_set_table(fs_builtin_ent *table, int cnt) {
	if (tbl != NULL) return -1;
	tbl = table;
	tblcnt = cnt;
	return 0;
}


/* Internal file structure; you can have eight files open at once on the
   built-in file system (for simplicity and to reduce malloc'ing). */
#define BI_FILES_MAX 8
typedef struct {
	int	tblidx;		/* Table entry */
	uint8	*data;		/* Quick access */
	uint32	size;		/* Ditto */
	off_t	ptr;		/* Offset ptr */
} fsb_fd_t;
static fsb_fd_t fsbtab[BI_FILES_MAX];

/* FD access mutex */
static spinlock_t mutex;

/* Standard file ops */
uint32 fsb_open(const char *fn, int mode) {
	int i, tblidx;

	/* Find the file entry (or not) */
	for (i=0; i<tblcnt; i++) {
		if (!stricmp(fn, tbl[i].name)) break;
	}
	if (i >= tblcnt) return 0;
	tblidx = i;

	/* Allocate an FD for it */
	spinlock_lock(&mutex);
	for (i=0; i<BI_FILES_MAX; i++) {
		if (fsbtab[i].tblidx == -1) {
			fsbtab[i].tblidx = tblidx;
			break;
		}
	}
	spinlock_unlock(&mutex);
	if (i >= BI_FILES_MAX) return 0;

	fsbtab[i].data = tbl[tblidx].data;
	fsbtab[i].size = tbl[tblidx].size;
	fsbtab[i].ptr = 0;

	return i+1;
}

void fsb_close(uint32 hnd) {
	if (hnd == 0) return;
	fsbtab[hnd-1].tblidx = -1;
}

ssize_t fsb_read(uint32 hnd, void *buf, size_t cnt) {
	if (hnd == 0 || fsbtab[hnd-1].tblidx == -1) return -1;
	hnd--;

	if ((fsbtab[hnd].ptr + cnt) >= fsbtab[hnd].size)
		cnt = fsbtab[hnd].size - fsbtab[hnd].ptr;
	if (cnt > 0)
		memcpy(buf, fsbtab[hnd].data + fsbtab[hnd].ptr, cnt);
	fsbtab[hnd].ptr += cnt;
	return cnt;
}

off_t fsb_seek(uint32 hnd, off_t offset, int whence) {
	if (hnd == 0 || fsbtab[hnd-1].tblidx == -1) return -1;
	hnd--;

	switch(whence) {
		case SEEK_SET: break;
		case SEEK_CUR: offset += fsbtab[hnd].ptr; break;
		case SEEK_END: offset = fsbtab[hnd].size - offset; break;
	}

	if (offset < 0)
		offset = 0;
	if (offset >= fsbtab[hnd].size)
		offset = fsbtab[hnd].size;
	fsbtab[hnd].ptr = offset;

	return fsbtab[hnd].ptr;
}

off_t fsb_tell(uint32 hnd) {
	if (hnd == 0 || fsbtab[hnd-1].tblidx == -1) return -1;
	return fsbtab[hnd-1].ptr;
}

size_t fsb_total(uint32 hnd) {
	if (hnd == 0 || fsbtab[hnd-1].tblidx == -1) return -1;
	return fsbtab[hnd-1].size;
}

void *fsb_mmap(uint32 hnd) {
	if (hnd == 0 || fsbtab[hnd-1].tblidx == -1) return NULL;
	return fsbtab[hnd-1].data;
}

/* Might implement this eventually */
#if 0
static dirent_t *fsb_readdir(uint32 hnd) {
}
#endif

/* Pull all that together */
static vfs_handler vh = {
	{ 0 },			/* Prefix */
	0, 0, NULL,		/* In-kernel, no cacheing, next */
	
	fsb_open,
	fsb_close,
	fsb_read,
	NULL,
	fsb_seek,
	fsb_tell,
	fsb_total,
	NULL,			/* readdir */
	NULL,			/* ioctl */
	NULL,			/* rename */
	NULL,			/* unlink */
	fsb_mmap
};

/* This is called before any main() gets to take a whack */
int fs_builtin_init() {
	int i;

	/* Start with no table */
	tbl = NULL; tblcnt = 0;

	/* Clear open files list */
	for (i=0; i<BI_FILES_MAX; i++)
		fsbtab[i].tblidx = -1;

	/* Init thread mutex */
	spinlock_init(&mutex);

	/* Register with VFS */
	return fs_handler_add("/bi", &vh);
}

int fs_builtin_shutdown() {
	return fs_handler_remove(&vh);
}

