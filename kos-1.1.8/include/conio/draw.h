/* KallistiOS 1.1.8

 draw.h

 (c)2002 Dan Potter

 Adapted from Kosh, (c)2000 Jordan DeLong

 draw.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp

*/
   
#ifndef __CONIO_DRAW_H
#define __CONIO_DRAW_H

/* font size constants */
#define CONIO_FONT_WIDTH 	12
#define CONIO_FONT_HEIGHT 	24

/* screen size constants */
#define CONIO_SCREEN_WIDTH	640
#define CONIO_SCREEN_HEIGHT	480

/* more constants */
#define CONIO_SCREEN_FIRSTCOL ( (CONIO_SCREEN_WIDTH - CONIO_FONT_WIDTH * CONIO_NUM_COLS) / 2 )
#define CONIO_SCREEN_LASTCOL  (CONIO_SCREEN_WIDTH - CONIO_SCREEN_FIRSTCOL)

/* functions */
void conio_draw_init();
void conio_draw_shutdown();
void conio_draw_frame();

#endif /* __CONIO_DRAW_H */

