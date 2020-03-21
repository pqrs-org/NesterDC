/* KallistiOS 1.1.8

   kernel/net/net_input.c
   (c)2002 Dan Potter
*/

#include <stdio.h>
#include <kos/net.h>
#include "net_icmp.h"

CVSID("net_input.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/*

  Main packet input system
 
*/

/* Where will input packets be routed? */
net_input_func net_input_target = net_icmp_input;

/* Process an incoming packet */
int net_input(netif_t *device, const uint8 *data, int len) {
	if (net_input_target != NULL)
		return net_input_target(device, data, len);
	else
		return 0;
}

/* Setup an input target; returns the old target */
net_input_func net_input_set_target(net_input_func t) {
	net_input_func old = net_input_target;
	net_input_target = t;
	return old;
}
