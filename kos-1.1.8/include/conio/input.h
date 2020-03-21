/* KallistiOS 1.1.8

 input.h

 (c)2002 Dan Potter

 Adapted from Kosh, (c)2000 Jordan DeLong

 input.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/

#ifndef __CONIO_INPUT_H
#define __CONIO_INPUT_H

/* size of the input buffer */
#define CONIO_INPUT_BUFFSIZE	256

/* functions */
void conio_input_frame();
void conio_input_init();
void conio_input_shutdown();

typedef void (*conio_input_callback_t)(const char *str);
void conio_input_callback(conio_input_callback_t cb);

/* Default conio input system: call with block = 1 to wait for the user
   to type something; the output will be placed in dst, which should
   be at least dstcnt bytes large. */
void conio_input_getline(int block, char *dst, int dstcnt);

#endif
