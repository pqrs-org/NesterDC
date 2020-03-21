/* KallistiOS 1.1.8

   jpeg.h
   (c)2001 Dan Potter

   jpeg.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/

#ifndef __JPEG_JPEG_H
#define __JPEG_JPEG_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <kos/img.h>

#ifdef _arch_dreamcast
	#include <dc/pvr.h>
	
	/* Load a JPEG file into a texture; returns 0 for success, -1 for failure. */
	int jpeg_to_texture(const char * filename, pvr_ptr_t tex, int size, int scale);
#endif

/* Load a JPEG into a KOS PIImage */
int jpeg_to_img(const char *filename, int scale, kos_img_t *rv);

__END_DECLS

#endif	/* __JPEG_JPEG_H */

