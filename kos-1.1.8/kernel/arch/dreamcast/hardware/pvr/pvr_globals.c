/* KallistiOS 1.1.8

   pvr_globals.c
   (C)2002 Dan Potter

 */

#include <dc/pvr.h>
#include "pvr_internal.h"

/*

  Global variables internal to the PVR module go here.
 
*/

CVSID("pvr_globals.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Our global state -- by default it's initialized to zeros, so the
   valid flag will be zero. */
volatile pvr_state_t pvr_state;


