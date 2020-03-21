/* KallistiOS 1.1.8

   fs_vmu.c
   (C)2000-2002 Jordan DeLong and Dan Potter

*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <arch/types.h>
#include <arch/spinlock.h>
#include <dc/fs_vmu.h>
#include <dc/maple.h>
#include <dc/maple/vmu.h>

CVSID("fs_vmu.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/*

This is the vmu filesystem module.  Because there are no directories on vmu's
it's pretty simple, however the filesystem uses a seperate directory for each
of the vmu slots, so if vmufs were mounted on /vmu, /vmu/a1/ is the dir for 
slot 1 on port a, and /vmu/c2 is slot 2 on port c, etc.

At the moment this FS is kind of a hack because of the simplicity (and weirdness)
of the VMU file system. For one, all files must be pretty small, so it loads
and caches the entire file on open. For two, all files are a multiple of 512
bytes in size (no way around this one). On top of it all, files may have an 
obnoxious header and you can't just read and write them with abandon like
a normal file system. We'll have to find ways around this later on, but for
now it gives the file data to you raw.

*/

#define VMU_DIR 	0
#define VMU_FILE	1

/* File handles */
typedef struct vmu_fh_str {
	uint32	strtype;		/* 0==dir, 1==file */
	struct	vmu_fh_str *next; 	/* next in the list */

	int	mode;			/* mode the file was opened with */
	char	path[16];		/* full path of the file */
	char	name[12];		/* name of the file */
	int32	diridx;			/* index in the vmu directory: -1 means it needs to be created when closed */
	off_t	loc;			/* current position in the file (bytes) */
	uint8	blk;			/* first block of this file */
	maple_device_t * dev;		/* maple address of the vmu to use */
	uint8	filesize;		/* file length from dirent (in 512-byte blks) */
	uint8	*data;			/* copy of the whole file */
} vmu_fh_t;

/* Directory handles */
typedef struct vmu_dh_str {
	uint32	strtype;		/* 0==dir, 1==file */
	struct	vmu_dh_str *next;	/* next in the list */

	dirent_t dirent;		/* Dirent to pass back */
	uint8	*dirblocks;		/* Copy of all directory blocks */
	uint16	entry;			/* Current dirent */
	uint16	dircnt;			/* Count of dir entries */	
	maple_device_t * dev;		/* VMU address */
} vmu_dh_t;

/* Linked list of open files (controlled by "mutex") */
static vmu_fh_t *vmu_fh = NULL;

/* Thread mutex for vmu_fh access */
static spinlock_t mutex;

/* Thread mutex for vmu writing operations */
static spinlock_t writemutex;

/* Prototypes */
int vmu_unlink(const char *path);

/* directory entries, 32 bytes each */
#define packed __attribute__((packed))
typedef struct {
	uint8	filetype	packed;	/* 0x00 = no file; 0x33 = data; 0xcc = a game */
	uint8	copyprotect	packed;	/* 0x00 = copyable; 0xff = copy protected */
	uint16	firstblk	packed;	/* location of the first block in the file */
	char	filename[12]	packed;	/* there is no null terminator */
	uint8	cent		packed;	/* these are all file creation date stuff, in BCD format */
	uint8	year		packed;
	uint8	month		packed;
	uint8	day		packed;
	uint8	hour		packed;
	uint8	min		packed;
	uint8	sec		packed;
	uint8	dow		packed;	/* day of week (0 = monday, etc) */
	uint16	filesize	packed;	/* size of the file in blocks */
	uint16	hdroff		packed;	/* offset of header, in blocks from start of file */
	uint8	dummys[4]	packed;	/* unused */
} directory_t;
#undef packed

/* Convert a decimal number to BCD; max of two digits */
static uint8 dec_to_bcd(int dec) {
	uint8 rv = 0;

	rv = dec % 10;
	rv |= ((dec / 10) % 10) << 4;

	return rv;
}

/* Fill in the date on a directory_t for writing */
static void vmu_fill_dir_time(directory_t *d) {
	time_t t;
	struct tm tm;

	/* Get the time */
	t = time(NULL);
	localtime_r(&t, &tm);

	/* Fill in the struct, converting to BCD */
	d->cent = 0x20;		/* Change if neccessary ;-) */
	d->year = dec_to_bcd(tm.tm_year - 100);
	d->month = dec_to_bcd(tm.tm_mon + 1);
	d->day = dec_to_bcd(tm.tm_mday);
	d->hour = dec_to_bcd(tm.tm_hour);
	d->min = dec_to_bcd(tm.tm_min);
	d->sec = dec_to_bcd(tm.tm_sec);
	d->dow = dec_to_bcd((tm.tm_wday - 1) % 7);
}

/* Take a VMUFS path and return the requested address */
static maple_device_t * vmu_path_to_addr(const char *p) {
	char port;

	if (p[0] != '/') return NULL;			/* Only absolute paths */
	port = p[1] | 32;				/* Lowercase the port */
	if (port < 'a' || port > 'd') return NULL;	/* Unit A-D, device 0-5 */
	if (p[2] < '0' || p[2] > '5') return NULL;
	
	return maple_enum_dev(port - 'a', p[2] - '0');
}

/* Find first directory entry matching "path", or first empty entry if path is NULL.
 * The root block (root_buf) must be read in before calling.
 * dir_buf will be filled with the matching directory block.
 * root_buf will be clobbered if it equals dir_buf.
 * Returns dirblock, diridx, and a directory_t * pointing somewhere in dir_buf.
 */
static directory_t *vmu_find_first(const char *path, maple_device_t * dev, uint8 *root_buf, uint8 *dir_buf, uint16 *dirblock, int32 *diridx) {
	directory_t *dir;
	uint16 dir_block;
	uint16 dir_len;
	int n, i;

	dir_block = ((uint16 *)root_buf)[0x4a/2];
	dir_len   = ((uint16 *)root_buf)[0x4c/2];

	/* search through the directory entries and find the first one with this filename */
	for (n = dir_len; n > 0; n--) {
		if (vmu_block_read(dev, dir_block, dir_buf) != 0) {
			dbglog(DBG_ERROR, "vmu_find_first: Can't read dir block %d\r\n", dir_block);
			break;
		}

		for (i = 0; i < 16; i++) {
			dir = (directory_t *) &dir_buf[i * 32];
			if (path) {
				if (dir->filetype) {
					if (strnicmp(&path[4], dir->filename, 12) == 0) {
						*diridx = i + (dir_len - n) * 16;
						*dirblock = dir_block;
						return dir;
					}
				}
			} else {
				if (dir->filetype == 0) {
					dbglog(DBG_KDEBUG, "vmu_find_first: got empty directory ent (%d, %d)\r\n", n, i);
					*diridx = i + (dir_len - n) * 16;
					*dirblock = dir_block;
					return dir;
				}
			}
		}
		dir_block--;
	}
	*dirblock = -1;
	*diridx = -1; /* may not be necessary due to NULL return */
	return NULL;
}

/* Open the fake vmu root dir /vmu */
vmu_fh_t *vmu_open_vmu_dir(void) {
	unsigned int p, u;
	unsigned int num = 0;
	unsigned short names[MAPLE_PORT_COUNT * MAPLE_UNIT_COUNT];
	vmu_dh_t *dh;
	directory_t *dir;

	/* Determine how many VMUs are connected */
	for (p=0; p<MAPLE_PORT_COUNT; p++) {
		for (u=0; u<MAPLE_UNIT_COUNT; u++) {
			if (maple_device_func(p, u) & MAPLE_FUNC_MEMCARD) {
				names[num++] = (('0' + u) << 8) + ('a' + p);  /* little endian */
				dbglog(DBG_KDEBUG, "vmu_open_vmu_dir: found memcard (%c%d)\n", 'a'+p, u);
			}
		}
	}

	dbglog(DBG_KDEBUG, "# of memcards found: %d\n", num);

	dh = malloc(sizeof(vmu_dh_t));
	dh->strtype = VMU_DIR;
	dh->dirblocks = malloc(num * 32);
	if (!dh->dirblocks) {
		free(dh);
		return(NULL);
	}
	dh->entry = 0;
	dh->dircnt = num;
	dh->dev = NULL;

	/* Create the directory entries */
	for (u=0; u<num; u++) {
		memset(dh->dirblocks + u * 32, 0, 32); 		/* Start in a clean room */
		dir = (directory_t *)(dh->dirblocks + u * 32);
		memcpy(dir->filename, &names[u], 2);
		dir->filetype = 0xff;
	}
	return (vmu_fh_t *)dh;
}

/* opendir function */
vmu_fh_t *vmu_open_dir(maple_device_t * dev) {
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	dirlength;	/* size of the directory in blocks */
	int	i, n;
	vmu_dh_t *dh;

	buff16 = (uint16*)buff;

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(dev, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\r\n", i);
		return 0;
	}
	dirblock = buff16[0x4a/2];
	dirlength = buff16[0x4c/2];

	/* Allocate a buffer for the dir blocks */
	dh = malloc(sizeof(vmu_dh_t));
	dh->strtype = VMU_DIR;
	dh->dirblocks = malloc(512*dirlength);
	if (!dh->dirblocks) {
		free(dh);
		return 0;
	}
	dh->entry = 0;
	dh->dircnt = (dirlength * 512) / 32;
	dh->dev = dev;
	
	/* Read all dir blocks */
	for (n=0; n<dirlength; n++) {
		if (vmu_block_read(dev, dirblock, dh->dirblocks + n*512) != 0) {
			dbglog(DBG_ERROR, "vmu_open_dir: Can't read dir block %d\r\n", dirblock);
			return 0;
		}
		dirblock--;
	}
	
	return (vmu_fh_t *)dh;
}

/* openfile function */
vmu_fh_t *vmu_open_file(maple_device_t * dev, const char *path, int mode) {
	vmu_fh_t *fd;		/* file descriptor */
	int32	diridx;		/* directory number */
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	fatblock;	/* the current FAT block */
	directory_t dir, *dir_ptr;
	int	i;

	buff16 = (uint16*)buff;

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(dev, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\r\n", i);
		return NULL;
	}

	dir_ptr = vmu_find_first(path, dev, buff, buff, &dirblock, &diridx);

	/* We need to initialize the directory entry if it doesn't exist,
	 * or if we're truncating the file. */
	if (diridx == -1 || (mode & O_TRUNC)) {
		/* if the file wasn't opened for writing we get upset now */
		if ((mode & O_MODE_MASK) != O_RDWR && (mode & O_MODE_MASK) != O_WRONLY) {
			return NULL;
		} else {
			dir.filesize = 1;
			dir.firstblk = 0;
		}
	} else {
		memcpy(&dir, dir_ptr, sizeof(directory_t));
	}

	/* malloc a new fh struct */
	fd = malloc(sizeof(vmu_fh_t));

	/* fill in the filehandle struct */
	fd->strtype = VMU_FILE;
	fd->diridx = diridx;
	fd->blk = dir.firstblk;
	fd->loc = 0;
	fd->dev = dev;
	fd->mode = mode;
	strncpy(fd->path, path, 16);
	strncpy(fd->name, &path[4], 12);
	fd->filesize = dir.filesize;
	if (fd->filesize > 200) {
		dbglog(DBG_WARNING, "VMUFS: file %s greater than 200 blocks: corrupt card?\r\n", path);
		free(fd);
		return NULL;
	}

	if (fd->filesize == 0) {
		dbglog(DBG_WARNING, "VMUFS: can't open zero-length file %s\r\n", path);
		free(fd);
		return NULL;
	}

	/* If the file doesn't exist or is newly truncated, we don't need to 
	 * read it, so we malloc 512 and exit now */
	if (fd->diridx == -1 || (mode & O_TRUNC)) {
		fd->data = malloc(512);
		memset(fd->data, 0, 512);
		return fd;
	}
	
	/* Read the FAT */
	if (vmu_block_read(dev, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %c%c)\r\n", 'a'+dev->port, '0'+dev->unit);
		free(fd);
		return NULL;
	}
	
	/* Follow the FAT, reading all blocks */
	fd->data = malloc(dir.filesize*512);
	fatblock = fd->blk;
	for (i=0; i<fd->filesize; i++) {
		dbglog(DBG_KDEBUG, "reading block: %d\r\n", fatblock);
		if (vmu_block_read(dev, fatblock, fd->data+i*512) < 0) {
			dbglog(DBG_ERROR, "Can't read block %d\r\n", fatblock);
			free(fd->data);
			free(fd);
			return NULL;
		}
		if (fatblock == 0xfffa && i < fd->filesize - 1) {
			dbglog(DBG_WARNING, "Warning: File shorter in FAT than DIR (%d vs %d)\r\n", i + 1, fd->filesize);
			fd->filesize = i + 1; 
			break;
		}
		fatblock = buff16[fatblock];
	}
	
	return fd;
}

/* open function */
uint32 vmu_open(const char *path, int mode) {
	maple_device_t	* dev;		/* maple bus address of the vmu unit */
	vmu_fh_t	*fh;

	if (!*path) {
		/* /vmu should be opened */
		fh = vmu_open_vmu_dir();
	} else {
		/* Figure out which vmu slot is being opened */
		dev = vmu_path_to_addr(path);
		/* printf("VMUFS: card address is %02x\r\n", addr); */
		if (dev == NULL) return 0;

		/* Check for open as dir */
		if (strlen(path) == 3) {
			if (!(mode & O_DIR)) return 0;
			fh = vmu_open_dir(dev);
		} else {
			fh = vmu_open_file(dev, path, mode);
		}
	}
	if (fh == NULL) return 0;

	/* link the fh onto the top of the list */
	spinlock_lock(&mutex);
	fh->next = vmu_fh;
	vmu_fh = fh;
	spinlock_unlock(&mutex);

	return (uint32)fh;
}

/* write a file out before closing it: we aren't perfect on error handling here */
int vmu_write_close(uint32 hnd) {
	uint8		buff[512];
	uint8		dirblockbuff[512];
	vmu_fh_t	*fh;
	directory_t	*dir;
	uint16		*buff16;
	uint16		dirblock, currblk;
	int32 		diridx;
	int		i, n, lastblk;

	fh = (vmu_fh_t*)hnd;

	if (fh->diridx != -1) {
		/* Existing file, unlink it first.  Using vmu_unlink results in
		 * some duplicated effort, but is clean. */
		/* There's a small window where someone could recreate the
		 * file, but then again, we're not protected against two
		 * threads opening/modifying the same file anyway. */
		/* Note: unlinking does not destroy our filehandle. */
		if (vmu_unlink(fh->path) == -1) {
			dbglog(DBG_ERROR, "vmu_write_close: failed to unlink existing file\n");
			return -1;
		}
	}

	/* for (i = 0; i < fh->filesize * 512; i++)
		printf("%c", fh->data[i]); */

	buff16 = (uint16 *) buff;

	spinlock_lock(&writemutex);

	/* read the root block and find out where the directory is and how long it is */
	if ((i = vmu_block_read(fh->dev, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "vmu_write_close: Can't read root block (%d)\r\n", i);
		goto return_error;
	}

	/* search through the directory entries and find the first free entry */
	dir = vmu_find_first(NULL, fh->dev, buff, dirblockbuff, &dirblock, &diridx);

	if (dir == NULL) {
		dbglog(DBG_ERROR, "vmu_write_close: No empty directory slots in VMU!\n");
		goto return_error;
	}

	/* add check for not getting empty direntry */

	/* Read the FAT */
	if (vmu_block_read(fh->dev, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %c%c)\r\n", 'a'+fh->dev->port, '0'+fh->dev->unit);
		goto return_error;
	}

	/* zero out the dir struct */
	memset(dir, 0, sizeof(directory_t));

	assert_msg(fh->filesize != 0, "vmu_write_close: can't write a zero-length file");

	/* Allocate blocks and write the data */
	lastblk = -1;
	for (n=0; n<fh->filesize; n++) {
		/* Find an open block, walking forward */
		for (currblk = lastblk + 1; currblk < 200; currblk++) {
			if (buff16[currblk] == 0xfffc) {
				break;
			}
		}

		/* Didn't find a free block? */
		if (currblk == 200) {
			/* Some file blocks may have been written to flash, but the FAT
			 * and directory are not modified and will not reflect this. */
			dbglog(DBG_ERROR, "vmu_write_close: no space left on device\n");
			goto return_error;
		}

		/* If this is the first block, set that in the header */
		if (lastblk == -1) {
			dir->firstblk = (uint16)currblk;
			dbglog(DBG_KDEBUG, "first user block: %d\n", dir->firstblk);
		}

		/* If this isn't the first block, then link the last block
		   in the chain to us. */
		if (lastblk != -1)
			buff16[lastblk] = currblk;

		/* Write out the block */
		dbglog(DBG_KDEBUG, "writing block %d to vmu at %d\n", n, currblk);
		if (vmu_block_write(fh->dev, currblk, fh->data + n*512) < 0) {
			dbglog(DBG_ERROR, "Can't write block %d of file to VMU\n", n);
			goto return_error;
		}

		lastblk = currblk;
	}

	/* Mark the last block as such in the FAT.  We disallow zero-length files,
	 * so lastblk is guaranteed to be valid. */
	buff16[lastblk] = 0xfffa;

	/* write the new fat */
	if (vmu_block_write(fh->dev, 254, buff) < 0) {
		dbglog(DBG_ERROR, "Can't write VMU FAT (address %c%c)\r\n", 'a'+fh->dev->port, '0'+fh->dev->unit);
		goto return_error;
	}

	/* fill in the rest of the directory entry */
	/* Currently, existing file attributes are ignored. */
	dir->filetype = 0x33;
	dir->copyprotect = 0x00;
	strncpy(dir->filename, fh->name, 12);
	dir->filesize = fh->filesize;
	dir->hdroff = 0;	/* Header is at file beginning */
	vmu_fill_dir_time(dir);

	/* write the new directory */
	if (vmu_block_write(fh->dev, dirblock, dirblockbuff) != 0) {
		dbglog(DBG_ERROR, "vmu_write_close: Can't write dir block %d\r\n", dirblock);
		goto return_error;
	}

	spinlock_unlock(&writemutex);

	return 0;

return_error:
	spinlock_unlock(&writemutex);
	return -1;
}

/* close a file */
void vmu_close(uint32 hnd) {
	vmu_fh_t *last, *cur;
	vmu_fh_t *fh;

	last = NULL;
	fh = (vmu_fh_t *)hnd;

	/* Look for the one to get rid of */
	spinlock_lock(&mutex);
	cur = vmu_fh;
	while (cur != NULL) {
		if (cur == fh) {
			if (last == NULL)
				vmu_fh = cur->next;
			else
				last->next = cur->next;
				
			switch (cur->strtype) {
				case VMU_DIR: {
					vmu_dh_t *dir = (vmu_dh_t*)cur;
					free(dir->dirblocks);
					break;
				}
				case VMU_FILE:
					/* if the file is open for writing we need to flush it to the vmu */
					if ((cur->mode & O_MODE_MASK) == O_WRONLY || 
					    (cur->mode & O_MODE_MASK) == O_RDWR)
						vmu_write_close(hnd);
					free(cur->data);
					break;
			}
			free(cur);
			break;
		}
		last = cur;
		cur = cur->next;
	}
	spinlock_unlock(&mutex);
}

/* Verify that a given hnd is actually in the list */
int vmu_verify_hnd(uint32 hnd, int type) {
	vmu_fh_t	*cur;
	int		rv;

	rv = 0;
	
	spinlock_lock(&mutex);
	cur = vmu_fh;
	while (cur != NULL) {
		if ((uint32)cur == hnd) {
			rv = 1;
			break;
		}
		cur = cur->next;
	}
	spinlock_unlock(&mutex);
	
	if (rv)
		return cur->strtype == type;
	else
		return 0;
}

/* read function */
ssize_t vmu_read(uint32 hnd, void *buffer, size_t cnt) {
	vmu_fh_t *fh;

	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	fh = (vmu_fh_t *)hnd;

	/* make sure we're opened for reading */
	if ((fh->mode & O_MODE_MASK) != O_RDONLY && (fh->mode & O_MODE_MASK) != O_RDWR)
		return 0;

	/* Check size */
	cnt = (fh->loc + cnt) > (fh->filesize*512) ?
		(fh->filesize*512 - fh->loc) : cnt;

	/* Reads past EOF return 0 */
	if ((long)cnt < 0)
		return 0;

	/* Copy out the data */
	memcpy(buffer, fh->data+fh->loc, cnt);
	fh->loc += cnt;
	
	return cnt;
}

/* write function */
ssize_t vmu_write(uint32 hnd, const void *buffer, size_t cnt) {
	vmu_fh_t	*fh;
	void		*tmp;
	int		n;

	/* Check the handle we were given */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	fh = (vmu_fh_t *)hnd;

	/* Make sure we're opened for writing */
	if ((fh->mode & O_MODE_MASK) != O_WRONLY && (fh->mode & O_MODE_MASK) != O_RDWR)
		return -1;

	/* Check to make sure we have enough room in data */
	if (fh->loc + cnt > fh->filesize * 512) {
		/* Figure out the new block count */
		n = ((fh->loc + cnt) - (fh->filesize * 512));
		if (n & 511)
			n = (n+512) & ~511;
		n = n / 512;

		dbglog(DBG_KDEBUG, "VMUFS: extending file's filesize by %d\n", n);
		
		/* We alloc another 512*n bytes for the file */
		tmp = realloc(fh->data, (fh->filesize + n) * 512);
		if (!tmp) {
			dbglog(DBG_ERROR, "VMUFS: unable to realloc another 512 bytes\r\n");
			return -1;
		}

		/* Assign the new pointer and clear out the new space */
		fh->data = tmp;
		memset(fh->data + fh->filesize * 512, 0, 512*n);
		fh->filesize += n;
	}

	/* insert the data in buffer into fh->data at fh->loc */
	/* memmove(&fh->data[fh->loc + cnt], &fh->data[fh->loc], cnt);
	memmove(&fh->data[fh->loc], buffer, cnt); */
	dbglog(DBG_KDEBUG, "VMUFS: adding %d bytes of data at loc %d (%d avail)\n",
		cnt, fh->loc, fh->filesize * 512);
	memcpy(fh->data + fh->loc, buffer, cnt);
	fh->loc += cnt;

	return cnt;
}

/* mmap a file */
/* note: writing past EOF will invalidate your pointer */
void *vmu_mmap(uint32 hnd) {
	vmu_fh_t *fh;

	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return NULL;

	fh = (vmu_fh_t *)hnd;

	return fh->data;
}

/* Seek elsewhere in a file */
off_t vmu_seek(uint32 hnd, off_t offset, int whence) {
	vmu_fh_t *fh;

	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	fh = (vmu_fh_t *)hnd;

	/* Update current position according to arguments */
	switch (whence) {
		case SEEK_SET: break;
		case SEEK_CUR: offset += fh->loc; break;
		case SEEK_END: offset = fh->filesize * 512 - offset; break;
		default:
			return -1;
	}
	
	/* Check bounds; allow seek past EOF. */
	if (offset < 0) offset = 0;
	fh->loc = offset;
	
	return fh->loc;
}

/* tell the current position in the file */
off_t vmu_tell(uint32 hnd) {
	/* Check the handle */
	if (!vmu_verify_hnd(hnd, VMU_FILE))
		return -1;

	return ((vmu_fh_t *) hnd)->loc;
}

/* return the filesize */
size_t vmu_total(uint32 fd) {
	/* Check the handle */
	if (!vmu_verify_hnd(fd, VMU_FILE))
		return -1;

	/* note that all filesizes are multiples of 512 for the vmu */
	return (((vmu_fh_t *) fd)->filesize) * 512;
}

/* read a directory handle */
dirent_t *vmu_readdir(uint32 fd) {
	vmu_dh_t	*dh;
	directory_t	*dir;

	/* Check the handle */
	if (!vmu_verify_hnd(fd, VMU_DIR)) {
		return NULL;
	}

	dh = (vmu_dh_t*)fd;

	/* printf("VMUFS: readdir on entry %d of %d\r\n", dh->entry, dh->dircnt); */

	/* Go to the next non-empty entry */
	while (dh->entry < dh->dircnt
		&& ((directory_t*)(dh->dirblocks + 32*dh->entry))->filetype == 0)
			dh->entry++;

	/* Check if we have any entries left */
	if (dh->entry >= dh->dircnt)
		return NULL;
	
	/* printf("VMUFS: reading non-null entry %d\r\n", dh->entry); */
		
	/* Ok, extract it and fill the dirent struct */
	dir = (directory_t*)(dh->dirblocks + 32*dh->entry);
	dh->dirent.size = dir->filesize*512;
	strncpy(dh->dirent.name, dir->filename, 12);
	dh->dirent.name[12] = 0;
	dh->dirent.time = 0;	/* FIXME */
	dh->dirent.attr = 0;

	/* Move to the next entry */
	dh->entry++;

	return &dh->dirent;
}

/* Delete a file (contributed by Brian Peek) */
int vmu_unlink(const char *path) {
	int32	diridx;		/* directory number */
	uint8 	buff[512]; 	/* for reading the directory entry and root block */
	uint16	*buff16;	/* 16-bit version of buff */
	uint16	dirblock;	/* the directory starting block */
	uint16	userblock;	/* the current user block */
	maple_device_t	* dev = NULL;	/* address of VMU */
	uint16	tmp;		/* store previous FAT entry */
	directory_t dir, *dir_ptr;
	int	i;

	/* convert path to valid VMU address */
	dev = vmu_path_to_addr(path);
	if (dev == NULL) {
		dbglog(DBG_ERROR, "VMUFS: vmu_unlink on invalid path '%s'\n", path);
		return -1;
	}

	buff16 = (uint16*)buff;

	spinlock_lock(&writemutex);

	/* read the root block and find out where the directory is and how long it is */
	if ( (i=vmu_block_read(dev, 255, buff)) != 0) {
		dbglog(DBG_ERROR, "VMUFS: Can't read root block (%d)\n", i);
		goto return_error;
	}

	dir_ptr = vmu_find_first(path, dev, buff, buff, &dirblock, &diridx);

	/* the direntry doesn't exist, so bail */
	if (diridx == -1) {
		dbglog(DBG_ERROR, "vmu_unlink: file not found: %s\n", path);
		goto return_error;
	}

	assert_msg(dir_ptr, "dir_ptr is NULL but diridx >= 0");

	memcpy(&dir, dir_ptr, sizeof(directory_t));

	if (dir.filesize > 200) {
		dbglog(DBG_WARNING, "VMUFS: file %s greater than 200 blocks: corrupt card?\n", path);
		goto return_error;
	}

	dbglog(DBG_KDEBUG, "clearing directory block: %d\n", dirblock);
	memset(dir_ptr, 0, sizeof(directory_t));
	if (vmu_block_write(dev, dirblock, buff) < 0) {
		dbglog(DBG_ERROR, "Can't write block %d\n", dirblock);
		goto return_error;
	}

	/* Read the FAT */
	if (vmu_block_read(dev, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't read VMU FAT (address %c%c)\n", 'a'+dev->port, '0'+dev->unit);
		goto return_error;
	}
	
	/* Follow the FAT, reading all blocks */
	userblock = dir.firstblk;

	for (i=0; i<dir.filesize; i++) {
		if (userblock >= 200) {
			/* Note: directory entry was cleared (so file will be gone) but
			 * space may not be fully recovered. */
			dbglog(DBG_WARNING, "vmu_unlink: block %d of %s reportedly at user block %d: corrupt card?\n", i, path, userblock);
			goto return_error;
		}
		dbglog(DBG_KDEBUG, "clearing user block: %d\r\n", userblock);
		if (userblock == 0xfffa && i < dir.filesize - 1) {
			dbglog(DBG_WARNING, "Warning: File shorter in FAT than DIR (%d vs %d)\n", i, dir.filesize);
			dir.filesize = i + 1; 
			goto return_error;
		}
		tmp = buff16[userblock];
		buff16[userblock] = 0xfffc;
		userblock = tmp;
	}
	
	/* Write the FAT */
	if (vmu_block_write(dev, 254, (uint8*)buff) < 0) {
		dbglog(DBG_ERROR, "Can't write VMU FAT (address %c%c)\n", 'a'+dev->port, '0'+dev->unit);
		goto return_error;
	}

	spinlock_unlock(&writemutex);
	return 0;

return_error:
	spinlock_unlock(&writemutex);
	return -1;
}

/* handler interface */
static vfs_handler vh = {
	{ "/vmu" },	/* path prefix */
	0, 1,		/* In-kernel, cache (not implemented yet, however) */
	NULL,		/* Linked list pointer */
	
	vmu_open,
	vmu_close,
	vmu_read,
	vmu_write,	/* the write function */
	vmu_seek,	/* the seek function */
	vmu_tell,
	vmu_total,
	vmu_readdir,	/* readdir */
	NULL,		/* ioctl */
	NULL,		/* rename/move */
	vmu_unlink,	/* unlink */		
	vmu_mmap	/* mmap */
};

int fs_vmu_init() {
	spinlock_init(&mutex);
	return fs_handler_add("/vmu", &vh);
}

int fs_vmu_shutdown() { return fs_handler_remove(&vh); }

