/* KallistiOS 1.1.8

   dc/biosfont.h
   (c)2000-2001 Dan Potter
   Japanese Functions (c)2002 Kazuaki Matsumoto

   biosfont.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp

*/


#ifndef __DC_BIOSFONT_H
#define __DC_BIOSFONT_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* Constants for the function below */
#define BFONT_CODE_ISO8859_1	0	/* DEFAULT */
#define BFONT_CODE_EUC		1
#define BFONT_CODE_SJIS		2

/* Select an encoding for Japanese (or disable) */
void bfont_set_encoding(int enc);

uint8 *bfont_find_char(int ch);
uint8 *bfont_find_char_jp(int ch);
uint8 *bfont_find_char_jp_half(int ch);

void bfont_draw(uint16 *buffer, int bufwidth, int opaque, int c);
void bfont_draw_thin(uint16 *buffer, int bufwidth, int opaque, int c, int iskana);
void bfont_draw_wide(uint16 *buffer, int bufwidth, int opaque, int c);
void bfont_draw_str(uint16 *buffer, int width, int opaque, char *str);

__END_DECLS

#endif  /* __DC_BIOSFONT_H */
