/* KallistiOS 1.1.8

   vmu.c
   (C)2002 Dan Potter
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <dc/maple.h>
#include <dc/maple/vmu.h>
#include <arch/timer.h>

CVSID("vmu.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/*
   This module deals with the VMU.  It provides functionality for
   memorycard access, and for access to the lcd screen.
  
   Thanks to Marcus Comstedt for VMU/Maple information.
 */

static int vmu_attach(maple_driver_t *drv, maple_device_t *dev) {
	dev->status_valid = 1;
	return 0;
}

static void vmu_detach(maple_driver_t *drv, maple_device_t *dev) {
}

/* Device Driver Struct */
static maple_driver_t vmu_drv = {
	functions:	MAPLE_FUNC_MEMCARD | MAPLE_FUNC_LCD | MAPLE_FUNC_CLOCK,
	name:		"VMU Driver",
	periodic:	NULL,
	attach:		vmu_attach,
	detach:		vmu_detach
};

/* Add the VMU to the driver chain */
int vmu_init() {
	return maple_driver_reg(&vmu_drv);
}

void vmu_shutdown() {
	maple_driver_unreg(&vmu_drv);
}

/* These interfaces will probably change eventually, but for now they
   can stay the same */

/* Draw a 1-bit bitmap on the LCD screen (48x32). return a -1 if
   an error occurs */
int vmu_draw_lcd(maple_device_t * dev, void *bitmap) {
	uint32 *	send_buf;

	assert( dev != NULL );
        
	/* Lock the frame */
	if (maple_frame_lock(&dev->frame) < 0)
		return MAPLE_EAGAIN;

	/* Reset the frame */
	maple_frame_init(&dev->frame);
	send_buf = (uint32 *)dev->frame.recv_buf;
	send_buf[0] = MAPLE_FUNC_LCD;
	send_buf[1] = 0;	/* Block / phase / partition */
	memcpy(send_buf+2, bitmap, 48 * 4);
	dev->frame.cmd = MAPLE_COMMAND_BWRITE;
	dev->frame.dst_port = dev->port;
	dev->frame.dst_unit = dev->unit;
	dev->frame.length = 2 + 48;
	dev->frame.callback = NULL;
	dev->frame.send_buf = send_buf;
	maple_queue_frame(&dev->frame);

	/* Wait for the LCD to accept it */
	while (dev->frame.state != MAPLE_FRAME_VACANT)
		;

	return MAPLE_EOK;
}

/* Read the data in block blocknum into buffer, return a -1
   if an error occurs, for now we ignore MAPLE_RESPONSE_FILEERR,
   which will be changed shortly */
static void vmu_block_read_callback(maple_frame_t *frm) {
	/* Do nothing -- we do this just to keep the frame locked */
}

int vmu_block_read(maple_device_t * dev, uint16 blocknum, uint8 *buffer) {
	maple_response_t	*resp;
	int			rv;
	uint32			*send_buf;
	uint32			blkid;

	assert( dev != NULL );

	/* Lock the frame */
	while (maple_frame_lock(&dev->frame) < 0)
		;
		
	/* this is (block << 24) | (phase << 8) | (partition (0 for all vmu)) */
	blkid = ((blocknum & 0xff) << 24) | ((blocknum >> 8) << 16);
	
	/* Reset the frame */
	maple_frame_init(&dev->frame);
	send_buf = (uint32 *)dev->frame.recv_buf;
	send_buf[0] = MAPLE_FUNC_MEMCARD;
	send_buf[1] = blkid;
	dev->frame.cmd = MAPLE_COMMAND_BREAD;
	dev->frame.dst_port = dev->port;
	dev->frame.dst_unit = dev->unit;
	dev->frame.length = 2;
	dev->frame.callback = vmu_block_read_callback;
	dev->frame.send_buf = send_buf;
	maple_queue_frame(&dev->frame);

	/* Wait for the VMU to accept it */
	while (dev->frame.state != MAPLE_FRAME_RESPONDED)
		;

	/* Copy out the response */
	resp = (maple_response_t *)dev->frame.recv_buf;
	send_buf = (uint32 *)resp->data;
	if (resp->response != MAPLE_RESPONSE_DATATRF
			|| send_buf[0] != MAPLE_FUNC_MEMCARD
			|| send_buf[1] != blkid) {
		rv = MAPLE_EFAIL;
		dbglog(DBG_ERROR, "vmu_block_read failed: %s(%d)/%08lx\r\n",
			maple_perror(resp->response), resp->response, send_buf[0]);
	} else {
		rv = MAPLE_EOK;
		memcpy(buffer, send_buf + 2, (resp->data_len - 2) * 4);
	}

	maple_frame_unlock(&dev->frame);

	return rv;
}

/* writes buffer into block blocknum.  ret a -1 on error.  We don't do anything about the
   maple bus returning file errors, etc, right now, but that will change soon. */
static void vmu_block_write_callback(maple_frame_t *frm) {
	frm->state = MAPLE_FRAME_UNSENT;
}
int vmu_block_write(maple_device_t * dev, uint16 blocknum, uint8 *buffer) {
	maple_response_t	*resp;
	int			rv, phase, r;
	uint32			*send_buf;
	uint32			blkid;

	assert( dev != NULL );

	rv = MAPLE_EOK;

	/* Lock the frame */
	while (maple_frame_lock(&dev->frame) < 0)
		;

	/* Writes have to occur in four phases per block -- this is the
	   way of flash memory, which you must erase an entire block 
	   at once to write; the blocks in this case are 128 bytes long. */
	for (phase=0; phase<4; phase++) {
		/* this is (block << 24) | (phase << 8) | (partition (0 for all vmu)) */
		blkid = ((blocknum & 0xff) << 24) | ((blocknum >> 8) << 16) | (phase << 8);
	
		/* Reset the frame */
		maple_frame_init(&dev->frame);
		send_buf = (uint32 *)dev->frame.recv_buf;
		send_buf[0] = MAPLE_FUNC_MEMCARD;
		send_buf[1] = blkid;
		memcpy(send_buf + 2, buffer + 128*phase, 128);
		dev->frame.cmd = MAPLE_COMMAND_BWRITE;
		dev->frame.dst_port = dev->port;
		dev->frame.dst_unit = dev->unit;
		dev->frame.length = 2 + (128 / 4);
		dev->frame.callback = vmu_block_write_callback;
		dev->frame.send_buf = send_buf;
		maple_queue_frame(&dev->frame);

		/* Wait for the VMU to accept it */
		while (dev->frame.queued || dev->frame.state != MAPLE_FRAME_UNSENT)
			;

		/* Check the response */
		resp = (maple_response_t *)dev->frame.recv_buf;
		r = resp->response;
		if (r != MAPLE_RESPONSE_OK) {
			rv = MAPLE_EFAIL;
			dbglog(DBG_ERROR, "Incorrect response writing phase %d:\n", phase);
			dbglog(DBG_ERROR, "response:      %s(%d)\n", maple_perror(resp->response), resp->response);
			dbglog(DBG_ERROR, "datalen:       %d\n", resp->data_len);
		}

		/* Gotta wait a bit for flash to catch up.. apparently maple doesn't
		   handle this for us */
		timer_spin_sleep(20);
	}

	/* Finally a "sync" command -- thanks Nagra */
	maple_frame_init(&dev->frame);
	send_buf = (uint32 *)dev->frame.recv_buf;
	send_buf[0] = MAPLE_FUNC_MEMCARD;
	send_buf[1] = ((blocknum & 0xff) << 24)
		| (((blocknum >> 8) & 0xff) << 16)
		| (4 << 8);
	dev->frame.cmd = MAPLE_COMMAND_BSYNC;
	dev->frame.dst_port = dev->port;
	dev->frame.dst_unit = dev->unit;
	dev->frame.length = 2;
	dev->frame.callback = NULL;
	dev->frame.send_buf = send_buf;
	maple_queue_frame(&dev->frame);

	/* Wait for the VMU to accept it */
	while (dev->frame.state != MAPLE_FRAME_VACANT)
		;

	timer_spin_sleep(20);

	return rv;
}

