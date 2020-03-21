/* KallistiOS 1.1.8

   fs_romdisk.c
   (c)2001 Dan Potter

*/

/*

This module implements the Linux ROMFS file system for reading a pre-made boot romdisk.

To create a romdisk image, you will need the program "genromfs". This program was designed
for Linux but ought to compile under Cygwin. The source for this utility can be found
on sunsite.unc.edu in /pub/Linux/system/recovery/, or as a package under Debian "genromfs".

*/

#include <arch/types.h>
#include <kos/thread.h>
#include <arch/spinlock.h>
#include <kos/fs_romdisk.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

CVSID("fs_romdisk.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Header definitions from Linux ROMFS documentation; all integer quantities are
   expressed in big-endian notation. Unfortunately the ROMFS guys were being
   clever and made this header a variable length depending on the size of
   the volume name *groan*. Its size will be a multiple of 16 bytes though. */
typedef struct {
	char	magic[8];		/* Should be "-rom1fs-" */
	uint32	full_size;		/* Full size of the file system */
	uint32	checksum;		/* Checksum */
	char	volume_name[16];	/* Volume name (zero-terminated) */
} romdisk_hdr_t;

/* File header info; note that this header plus filename must be a multiple of
   16 bytes, and the following file data must also be a multiple of 16 bytes. */
typedef struct {
	uint32	next_header;		/* Offset of next header */
	uint32	spec_info;		/* Spec info */
	uint32	size;			/* Data size */
	uint32	checksum;		/* File checksum */
	char	filename[16];		/* File name (zero-terminated) */
} romdisk_file_t;


/* Util function to reverse the byte order of a uint32 */
static uint32 ntohl_32(const void *data) {
	const uint8 *d = (const uint8*)data;
	return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3] << 0);
}

/* Pointer to the ROMFS image created by the host PC */
static uint8 *romdisk_image = NULL;

/* Some convienence pointers */
static romdisk_hdr_t *romdisk_hdr = NULL;
static uint32 romdisk_files = 0;

/********************************************************************************/
/* File primitives */

/* File handles.. I could probably do this with a linked list, but I'm just
   too lazy right now. =) */
static struct {
	uint32		index;		/* romfs image index */
	int		dir;		/* >0 if a directory */
	uint32		ptr;		/* Current read position in bytes */
	uint32		size;		/* Length of file in bytes */
	dirent_t	dirent;		/* A static dirent to pass back to clients */
} fh[MAX_RD_FILES];

/* Mutex for file handles */
static spinlock_t fh_mutex;

/* Given a filename and a starting romdisk directory listing (byte offset),
   search for the entry in the directory and return the byte offset to its
   entry. */
static uint32 romdisk_find_object(const char *fn, int fnlen, int dir, uint32 offset) {
	uint32		i, ni, type;
	romdisk_file_t	*fhdr;

	i = offset;
	do {
		/* Locate the entry, next pointer, and type info */
		fhdr = (romdisk_file_t *)(romdisk_image + i);
		ni = ntohl_32(&fhdr->next_header);
		type = ni & 0x0f;
		ni = ni & 0xfffffff0;

		/* Check the type */
		if (!dir) {
			if ((type & 3) != 2) {
				i = ni;
				if (!i)
					break;
				else
					continue;
			}
		} else {
			if ((type & 3) != 1) {
				i = ni;
				if (!i)
					break;
				else
					continue;
			}
		}
		
		/* Check filename */
		if (!strncasecmp(fhdr->filename, fn, fnlen)) {
			/* Match: return this index */
			return i;
		}
		
		i = ni;
	} while (i != 0);

	/* Didn't find it */
	return 0;
}

/* Locate an object anywhere in the image, starting at the root, and
   expecting a fully qualified path name. This is analogous to the
   find_object_path in iso9660.

   fn:		object filename (absolute path)
   dir:		0 if looking for a file, 1 if looking for a dir

   It will return an offset in the romdisk image for the object. */
static uint32 romdisk_find(const char *fn, int dir) {
	const char	*cur;
	uint32		i;
	romdisk_file_t	*fhdr;

	/* If the object is in a sub-tree, traverse the trees looking
	   for the right directory. */
	i = romdisk_files;
	while ((cur = strchr(fn, '/'))) {
		if (cur != fn) {
			i = romdisk_find_object(fn, cur - fn, 1, i);
			if (i == 0) return 0;
			fhdr = (romdisk_file_t *)(romdisk_image + i);
			i = ntohl_32(&fhdr->spec_info);
		}
		fn = cur+1;
	}

	/* Locate the file in the resulting directory */
	if (*fn) {
		i = romdisk_find_object(fn, strlen(fn), dir, i);
		return i;
	} else {
		if (!dir)
			return 0;
		else
			return i;
	}
}

/* Open a file or directory */
uint32 romdisk_open(const char *fn, int mode) {
	uint32		fd;
	uint32		filehdr;
	romdisk_file_t	*fhdr;

	/* Make sure they don't want to open things as writeable */
	if ((mode & O_MODE_MASK) != O_RDONLY)
		return 0;
	
	/* No blank filenames */
	if (fn[0] == 0)
		fn = "";

	/* Look for the file */
	filehdr = romdisk_find(fn + 1, mode & O_DIR);
	if (filehdr == 0)
		return 0;

	/* Find a free file handle */
	spinlock_lock(&fh_mutex);
	for (fd=0; fd<MAX_RD_FILES; fd++)
		if (fh[fd].index == 0) {
			fh[fd].index = -1;
			break;
		}
	spinlock_unlock(&fh_mutex);
	if (fd >= MAX_RD_FILES)
		return 0;
	
	/* Fill the fd structure */
	fhdr = (romdisk_file_t *)(romdisk_image + filehdr);
	fh[fd].index = filehdr + sizeof(romdisk_file_t) + (strlen(fhdr->filename)/16)*16;
	fh[fd].dir = (mode & O_DIR) ? 1 : 0;
	fh[fd].ptr = 0;
	fh[fd].size = ntohl_32(&fhdr->size);
	
	return fd;
}

/* Close a file or directory */
void romdisk_close(uint32 fd) {
	/* Check that the fd is valid */
	if (fd < MAX_RD_FILES) {
		/* No need to lock the mutex: this is an atomic op */
		fh[fd].index = 0;
	}
}

/* Read from a file */
ssize_t romdisk_read(uint32 fd, void *buf, size_t bytes) {
	/* Check that the fd is valid */
	if (fd >= MAX_RD_FILES || fh[fd].index == 0 || fh[fd].dir)
		return -1;

	/* Is there enough left? */
	if ((fh[fd].ptr + bytes) > fh[fd].size)
		bytes = fh[fd].size - fh[fd].ptr;

	/* Copy out the requested amount */
	memcpy(buf, romdisk_image + fh[fd].index + fh[fd].ptr, bytes);
	fh[fd].ptr += bytes;

	return bytes;
}

/* Seek elsewhere in a file */
off_t romdisk_seek(uint32 fd, off_t offset, int whence) {
	/* Check that the fd is valid */
	if (fd>=MAX_RD_FILES || fh[fd].index==0 || fh[fd].dir)
		return -1;

	/* Update current position according to arguments */
	switch (whence) {
		case SEEK_SET:
			fh[fd].ptr = offset;
			break;
		case SEEK_CUR:
			fh[fd].ptr += offset;
			break;
		case SEEK_END:
			fh[fd].ptr = fh[fd].size + offset;
			break;
		default:
			return -1;
	}
	
	/* Check bounds */
	if (fh[fd].ptr < 0) fh[fd].ptr = 0;
	if (fh[fd].ptr > fh[fd].size) fh[fd].ptr = fh[fd].size;
	
	return fh[fd].ptr;
}

/* Tell where in the file we are */
off_t romdisk_tell(uint32 fd) {
	if (fd>=MAX_RD_FILES || fh[fd].index==0 || fh[fd].dir)
		return -1;

	return fh[fd].ptr;
}

/* Tell how big the file is */
size_t romdisk_total(uint32 fd) {
	if (fd>=MAX_RD_FILES || fh[fd].index==0 || fh[fd].dir)
		return -1;

	return fh[fd].size;
}

/* Read a directory entry */
dirent_t *romdisk_readdir(uint32 fd) {
	romdisk_file_t	*fhdr;
	int		type;
	
	if (fd>=MAX_RD_FILES || fh[fd].index==0 || !fh[fd].dir)
		return NULL;

	if (fh[fd].index == 0)
		return NULL;

	/* Get the current file header */
	fhdr = (romdisk_file_t *)(romdisk_image + fh[fd].index + fh[fd].ptr);

	/* Update the pointer */
	fh[fd].ptr = ntohl_32(&fhdr->next_header);
	type = fh[fd].ptr & 0x0f;
	fh[fd].ptr = fh[fd].ptr & 0xfffffff0;
	if (fh[fd].ptr == 0)
		fh[fd].index = 0;
	else
		fh[fd].ptr = fh[fd].ptr - fh[fd].index;

	/* Copy out the requested data */
	strcpy(fh[fd].dirent.name, fhdr->filename);
	fh[fd].dirent.time = 0;
	if ((type & 3) == 1) {
		fh[fd].dirent.attr = O_DIR;
		fh[fd].dirent.size = -1;
	} else {
		fh[fd].dirent.attr = 0;
		fh[fd].dirent.size = ntohl_32(&fhdr->size);
	}

	return &fh[fd].dirent;
}

void *romdisk_mmap(uint32 fd) {
	if (fd>=MAX_RD_FILES || fh[fd].index==0)
		return NULL;

	return romdisk_image + fh[fd].index;
}

/* Put everything together */
static vfs_handler vh = {
	{ "romdisk" },		/* name */
	0, 0, NULL,		/* In-kernel, no cacheing, next */
	romdisk_open,
	romdisk_close,
	romdisk_read,
	NULL,
	romdisk_seek,
	romdisk_tell,
	romdisk_total,
	romdisk_readdir,
	NULL,
	NULL,
	NULL,
	romdisk_mmap
};

/* Initialize the file system */
int fs_romdisk_init(uint8 *img) {
	/* int i, ni;
	romdisk_file_t *fhdr; */
	
	/* Set the ROMFS image */
	romdisk_image = img;
	
	/* Check and print some info about it */
	romdisk_hdr = (romdisk_hdr_t *)romdisk_image;
	if (strncmp(romdisk_image, "-rom1fs-", 8)) {
		dbglog(DBG_ERROR, "Rom disk image at 0x%x is not a ROMFS image\r\n", img);
		return -1;
	}
	dbglog(DBG_NOTICE, "ROMFS image recognized. Full size is 0x%x bytes\r\n",
		ntohl_32(&romdisk_hdr->full_size));
	dbglog(DBG_KDEBUG, "  Checksum is 0x%x\r\n",
		ntohl_32(&romdisk_hdr->checksum));
	dbglog(DBG_KDEBUG, "  Volume ID is ``%s''\r\n",
		romdisk_hdr->volume_name);

	romdisk_files = sizeof(romdisk_hdr_t)
		+ (strlen(romdisk_hdr->volume_name) / 16) * 16;
	dbglog(DBG_KDEBUG, "  File entries begin at offset 0x%x\r\n",
		romdisk_files);

	/* printf("Files:\r\n");
	i = romdisk_files;
	do {
		printf("0x%x: ", i);
		
		fhdr = (romdisk_file_t*)(romdisk_image + i);
		ni = ntohl_32(&fhdr->next_header);
		printf("next=%x, ", ni & 0xfffffff0);
		printf("type=%x, ", ni & 0x0f);
		ni &= 0xfffffff0;
		printf("spec_info=%x, ", ntohl_32(&fhdr->spec_info));
		printf("size=%x, ", ntohl_32(&fhdr->size));
		printf("checksum=%x ", ntohl_32(&fhdr->checksum));
		printf("filename='%s'\r\n", fhdr->filename);
		printf("  File data starts at %x\r\n",
			i + sizeof(romdisk_file_t)
			+ (strlen(fhdr->filename) / 16) * 16);
		
		i = ni;
	} while (i != 0); */

	/* Reset fd's */
	memset(fh, 0, sizeof(fh));
	
	/* Mark the first as active so we can have an error FD of zero */
	fh[0].index = -1;
	
	/* Init thread mutexes */
	spinlock_init(&fh_mutex);

	/* Register with VFS */
	return fs_handler_add("/rd", &vh);
}

/* De-init the file system */
int fs_romdisk_shutdown() {
	return fs_handler_remove(&vh);
}



