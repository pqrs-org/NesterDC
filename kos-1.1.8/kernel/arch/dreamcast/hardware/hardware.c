/* KallistiOS 1.1.8

   hardware.c
   (c)2000-2001 Dan Potter
*/

#include <arch/arch.h>
#include <dc/spu.h>
#include <dc/video.h>
#include <dc/cdrom.h>
#include <dc/asic.h>
#include <dc/maple.h>
#include <dc/net/broadband_adapter.h>
#include <dc/net/lan_adapter.h>

CVSID("hardware.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

static int initted = 0;

int hardware_init() {
	/* Init sound */
	spu_init();

	/* Init CD-ROM.. NOTE: NO GD-ROM SUPPORT. ONLY CDs/CDRs. */
	cdrom_init();

	/* Setup ASIC stuff */
	asic_init();

	/* Setup maple bus */
	maple_init();

	/* Init video */
	vid_init(DEFAULT_VID_MODE, DEFAULT_PIXEL_MODE);

	/* Setup network (this won't do anything unless we enable netcore) */
	bba_init();
	la_init();

	initted = 1;

	return 0;
}

void hardware_shutdown() {
	if (!initted)
		return;
	la_shutdown();
	bba_shutdown();
	maple_shutdown();
	asic_shutdown();
	cdrom_shutdown();
	spu_shutdown();
	vid_shutdown();
}


