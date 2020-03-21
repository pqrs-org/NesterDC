/* KallistiOS 1.1.8

   net_icmp.h
   (c)2002 Dan Potter

   net_icmp.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp

*/

#ifndef __LOCAL_NET_ICMP_H
#define __LOCAL_NET_ICMP_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <kos/net.h>

int net_icmp_input(netif_t *src, uint8 *pkt, int pktsize);

__END_DECLS

#endif	/* __LOCAL_NET_ICMP_H */

