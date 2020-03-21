/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

// manual override
//#define NESTER_DEBUG

#ifndef __DREAMCAST__
#  include <stdio.h>
#  define LOG(MSG) printf("%s\n", MSG);
#  define LOGHEX(X) printf("%x\n", X);
#  define ERROR(MSG) fprintf(stderr, "[ERROR] %s\n", MSG);
#  define CRASH_POINT()
#  if 0
#    include <assert.h>
#    define ASSERT(EXPR) assert(EXPR)
#  else
#    define ASSERT(EXPR)
#  endif
#else
#  include "dc_utils.h"
#  define LOG(MSG)
#  define LOGHEX(X)
#  define ERROR(MSG) { dc_pvr_font_output_message(NULL, MSG, NULL, NULL); }
#  define CRASH_POINT() \
  { \
    char hoge[128]; \
    timer_spin_sleep (*((int *)(hoge + 1))); \
  }
#  define ASSERT(EXPR)
#endif

#endif

