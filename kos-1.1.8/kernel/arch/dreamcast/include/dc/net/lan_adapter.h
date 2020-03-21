/* KallistiOS 1.1.8
 *
 * dc/net/lan_adapter.h
 *
 * (c)2002 Dan Potter
 *
 * lan_adapter.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp
 */

#ifndef __DC_NET_LAN_ADAPTER_H
#define __DC_NET_LAN_ADAPTER_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <kos/net.h>

extern struct netif la_if;

/* Initialize */
int la_init();

/* Shutdown */
int la_shutdown();

__END_DECLS

#endif	/* __DC_NET_LAN_ADAPTER_H */

