/* KallistiOS 1.1.8

   fs.c
   (c)2000-2001 Dan Potter

*/

/*

This module manages all of the file system code. Basically the VFS works
something like this:

- The kernel contains path primitives. There is a table of VFS path handlers
  installed by loaded servers. When the kernel needs to open a file, it will
  search this path handler table from the bottom until it finds a handler
  that is willing to take the request. The request is then handed off to
  the handler.
- The path handler receives the part of the path that is left after the
  part in the handler table. The path handler should return an internal
  handle for accessing the file. An internal handle of zero is always 
  assumed to mean failure.
- The kernel open function takes this value and wraps it in a structure that
  describes which service handled the request, and its internal handle.
- Subsequent operations go through this abstraction layer to land in the
  right place.

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <kos/fs.h>
#include <kos/thread.h>
#include <arch/spinlock.h>

CVSID("fs.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Thread mutex */
static spinlock_t mutex;

/* File handler structures; these structs contain path/function pairs that 
   describe how to handle a given path name. This is used mainly during
   the open process. */
typedef vfs_handler fs_handler_t;
static fs_handler_t *fs_handlers;

/* File handle structure; this is an entirely internal structure so it does
   not go in a header file. */
typedef struct {
	vfs_handler	*handler;	/* Handler */
	file_t		hnd;		/* Handler-internal */
} fs_hnd_t;


/* Internal file commands for root dir reading */
static file_t fs_root_opendir() {
	fs_hnd_t	*hnd;

	hnd = malloc(sizeof(fs_hnd_t));
	hnd->handler = NULL;
	hnd->hnd = 0;
	return (file_t)hnd;
}

/* Not thread-safe right now */
static dirent_t root_readdir_dirent;
static dirent_t *fs_root_readdir(file_t hnd) {
	fs_hnd_t	*handle;
	fs_handler_t	*handler;
	int 		cnt;

	handle = (fs_hnd_t*)hnd;
	handler = fs_handlers;
	cnt = handle->hnd;
	
	while (handler != NULL && cnt--)
		handler = handler->next;
	
	if (handler == NULL)
		return NULL;

	root_readdir_dirent.size = -1;
	if (handler->prefix[0] == '/')
		strcpy(root_readdir_dirent.name, handler->prefix+1);
	else
		strcpy(root_readdir_dirent.name, handler->prefix);
	handle->hnd++;
	return &root_readdir_dirent;
}

/* Locate a file handler for a given path name */
static fs_handler_t *fs_get_handler(const char *fn) {
	fs_handler_t	*cur;

	cur = fs_handlers;

	/* Scan the handler table and look for a path match */
	while (cur != NULL) {
		if (!strnicmp(cur->prefix, fn, strlen(cur->prefix)))
			break;
		cur = cur->next;
	}
	if (cur == NULL) {
		/* Couldn't find a handler */
		return NULL;
	}
	else
		return cur;
}

/* Resolve a relative path into a regular path */
static void fs_resolve_relative(const char *fn, char *rfn) {
	/* For now we don't do anything but tack on the process'
	   pwd to the front of the filename. Later we need to do
	   resolution of '..' and so on. */
	const char *root;

	root = thd_get_pwd(thd_get_current());

	/* Copy in the path names, careful of buffer limitations */		
	strncpy(rfn, root, 255);
	strcat(rfn, "/");
	strncat(rfn, fn, (255 - strlen(rfn)) - 1);
}

/* Attempt to open a file, given a path name. Follows the process described
   in the above comments. */
uint32 fs_open(const char *fn, int mode) {
	fs_handler_t	*cur;
	const char	*cname;
	uint32		h;
	fs_hnd_t	*hnd;
	char		rfn[256];

	/* Are they trying to open the root? */
	if (!strcmp(fn, "/")) {
		if ((mode & O_DIR)) 
			return fs_root_opendir();
		else
			return 0;
	}

	/* Are they using a relative path? */
	if (fn[0] != '/') {
		fs_resolve_relative(fn, rfn);
		fn = (const char *)rfn;
	}

	/* Look for a handler */
	cur = fs_get_handler(fn);
	if (cur == NULL)
		return 0;

	/* Found one -- get the "canonical" path name */
	cname = fn + strlen(cur->prefix);
	/* printf("Got a handler, canonical name is '%s'\r\n", cname); */

	/* Invoke the handler */
	if (cur->open == NULL) return 0;
	/*printf("Handler for open is ok\r\n");*/
	h = cur->open(cname, mode);
	if (h == 0) return 0;
	/*printf("Past check\r\n");*/

	/* Wrap it up in a structure */
	hnd = malloc(sizeof(fs_hnd_t));
	if (hnd == NULL) {
		cur->close(h);
		return 0;
	}
	hnd->handler = cur;
	hnd->hnd = h;

	return (uint32)hnd;
}

/* Close a file and clean up the handle */
void fs_close(uint32 hnd) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0) return;
	if (h->handler != NULL) {
		if (h->handler->close == NULL) return;
		h->handler->close(h->hnd);
	}
	free(h);
}

/* The rest of these pretty much map straight through */
ssize_t fs_read(uint32 hnd, void *buffer, size_t cnt) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->read == NULL) return -1;
	return h->handler->read(h->hnd, buffer, cnt);
}

ssize_t fs_write(uint32 hnd, const void *buffer, size_t cnt) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->write == NULL) return -1;
	return h->handler->write(h->hnd, buffer, cnt);
}

off_t fs_seek(uint32 hnd, off_t offset, int whence) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->seek == NULL) return -1;
	return h->handler->seek(h->hnd, offset, whence);
}

off_t fs_tell(uint32 hnd) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->tell == NULL) return -1;
	return h->handler->tell(h->hnd);
}

size_t fs_total(uint32 hnd) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->total == NULL) return -1;
	return h->handler->total(h->hnd);
}

dirent_t *fs_readdir(uint32 hnd) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0) return NULL;
	if (h->handler == NULL) return fs_root_readdir(hnd);
	if (h->handler->readdir == NULL) return NULL;
	return h->handler->readdir(h->hnd);
}

int fs_ioctl(uint32 hnd, void *data, size_t size) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->ioctl == NULL) return -1;
	return h->handler->ioctl(h->hnd, data, size);
}

int fs_rename(const char *fn1, const char *fn2) {
	fs_handler_t	*fh1, *fh2;
	
	/* Look for handlers */
	fh1 = fs_get_handler(fn1);
	if (fh1 == NULL) return -1;
	
	fh2 = fs_get_handler(fn2);
	if (fh2 == NULL) return -1;
	
	if (fh1 != fh2) return -2;

	if (fh1->rename)
		return fh1->rename(fn1+strlen(fh1->prefix),
			fn2+strlen(fh1->prefix));
	else
		return -3;
}

int fs_unlink(const char *fn) {
	fs_handler_t	*cur;
	
	/* Look for a handler */
	cur = fs_get_handler(fn);
	if (cur == NULL) return 1;

	if (cur->unlink)	
		return cur->unlink(fn+strlen(cur->prefix));
	else
		return -1;
}

int fs_chdir(const char *fn) {
	thd_set_pwd(thd_get_current(), fn);
	return 0;
}

const char *fs_getwd() {
	return thd_get_pwd(thd_get_current());
}

void *fs_mmap(file_t hnd) {
	fs_hnd_t *h = (fs_hnd_t*)hnd;

	if (hnd == 0 || h->handler == NULL || h->handler->mmap == NULL) return NULL;
	return h->handler->mmap(h->hnd);
}


/* Add a VFS module */
int fs_handler_add(const char *prefix, vfs_handler *hnd) {
	spinlock_lock(&mutex);

	strcpy(hnd->prefix, prefix);
	hnd->next = fs_handlers;
	fs_handlers = hnd;

	spinlock_unlock(&mutex);

	return 0;
}

/* Remove a VFS module */
int fs_handler_remove(const vfs_handler *hnd) {
	fs_handler_t *last, *cur;

	spinlock_lock(&mutex);

	last = NULL;
	cur = fs_handlers;
	while (cur != NULL) {
		if (cur == hnd) {
			break;
		}
		last = cur;
		cur = cur->next;
	}
	if (cur == NULL) {
		spinlock_unlock(&mutex);
		return -1;
	}

	if (last == NULL)
		fs_handlers = cur->next;
	else
		last->next = cur->next;

	spinlock_unlock(&mutex);

	return 0;
}

/* Initialize FS structures */
int fs_init() {
	int rv = 0;

	/* Start with no handlers */
	fs_handlers = NULL;

	/* Init thread mutex */
	spinlock_init(&mutex);

	return rv;
}

void fs_shutdown() {
	fs_handler_t *cur, *tmp;

	cur = fs_handlers;
	while (cur != NULL) {
		tmp = cur->next;
		free(cur);
		cur = tmp;
	}
}
