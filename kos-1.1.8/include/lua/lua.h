/* KallistiOS 1.1.8

   lua.h
   (c)2002 Dan Potter

   lua.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
*/

#ifndef __LUA_LUA_H
#define __LUA_LUA_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/* Just include the stuff from the Lua tree */
#include "../../addons/lua/include/lua.h"
#include "../../addons/lua/include/lauxlib.h"
#include "../../addons/lua/include/luadebug.h"
#include "../../addons/lua/include/lualib.h"

/* And our fputs shim */
void luaB_set_fputs(void (* fputs_target)(const char *));

__END_DECLS

#endif	/* __LUA_LUA_H */


