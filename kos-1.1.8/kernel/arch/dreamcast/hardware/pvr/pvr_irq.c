/* KallistiOS 1.1.8

   pvr_irq.c
   (C)2002 Dan Potter

 */

#include <assert.h>
#include <dc/pvr.h>
#include <dc/video.h>
#include <dc/asic.h>
#include "pvr_internal.h"

/*
   PVR interrupt handler; the way things are setup, we're gonna get
   one of these for each full vertical refresh and at the completion
   of TA data acceptance. The timing here is pretty critical. We need
   to flip pages during a vertical blank, and then signal to the program
   that it's ok to start playing with TA registers again, or we waste
   rendering time.
*/

CVSID("pvr_irq.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

void pvr_int_handler(uint32 code) {
	/* What kind of event did we get? Is it a list completion? */
	switch(code) {
	case ASIC_EVT_PVR_OPAQUEDONE:
		pvr_state.opb_completed |= 1 << PVR_OPB_OP;
		return;
	case ASIC_EVT_PVR_TRANSDONE:
		pvr_state.opb_completed |= 1 << PVR_OPB_TP;
		return;
	case ASIC_EVT_PVR_OPAQUEMODDONE:
		pvr_state.opb_completed |= 1 << PVR_OPB_OM;
		return;
	case ASIC_EVT_PVR_TRANSMODDONE:
		pvr_state.opb_completed |= 1 << PVR_OPB_TM;
		return;
	case ASIC_EVT_PVR_PTDONE:
		pvr_state.opb_completed |= 1 << PVR_OPB_PT;
		break;
	case ASIC_EVT_PVR_RENDERDONE:	/* never gets called sometimes?! */
		/* vid_border_color(255,255,0); */
		pvr_state.render_completed = 1;
		break;
        case ASIC_EVT_PVR_SCANINT1:
		break;
	}

	/* Vertical blank is happening, all lists are complete, and render is done */
	/* Note: render-done doesn't seem to be reliable */
	if (pvr_state.opb_completed == pvr_state.opb_completed_full
			&& pvr_state.render_completed
			/* && !to_texture */
			&& code == ASIC_EVT_PVR_SCANINT1) {
		/* Handle PVR stats */
		pvr_sync_stats();

		/* Switch view address to the "good" buffer */
		pvr_state.view_page ^= 1;
		pvr_sync_view_page();
		
		/* Finish up rendering the current frame (into the other buffer) */
		/* vid_border_color(0,0,255); */
		pvr_state.render_completed = 0;
		pvr_begin_queued_render();
		
		/* Mark render/flip completion */
		pvr_state.flip_completed = 1;
		
		/* Clear list completion */
		pvr_state.opb_completed = 0;

		/* Setup registration for the next frame */
		pvr_sync_reg_buffer();
	}
}


