/* KallistiOS 1.1.8

   dc/maple/vmu.h
   (C)2000-2002 Jordan DeLong and Dan Potter

   vmu.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp

*/

#ifndef __DC_MAPLE_VMU_H
#define __DC_MAPLE_VMU_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

int vmu_draw_lcd(maple_device_t * dev, void *bitmap);
int vmu_block_read(maple_device_t * dev, uint16 blocknum, uint8 *buffer);
int vmu_block_write(maple_device_t * dev, uint16 blocknum, uint8 *buffer);

/* Init / Shutdown */
int vmu_init();
void vmu_shutdown();

__END_DECLS

#endif	/* __DC_MAPLE_VMU_H */

