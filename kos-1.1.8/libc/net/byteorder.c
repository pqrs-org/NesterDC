/* KallistiOS 1.1.8

   byteorder.c
   (c)2001 Dan Potter

   byteorder.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
 */

/* Byte-order translation functions */
#include <arch/types.h>

/* XXX needs to be in arch */

/* Network to Host short */
uint16 ntohs(uint16 value) {
	return ((value >> 8) & 0xff)
		| ((value << 8) & 0xff00);
}

/* Network to Host long */
uint32 ntohl(uint32 value) {
	return ((value >> 24) & 0xff)
		| (( (value >> 16) & 0xff ) << 8)
		| (( (value >> 8) & 0xff) << 16)
		| (( (value >> 0) & 0xff) << 24);
}

/* Host to Network short */
uint32 htons(uint32 value) { return ntohs(value); }

/* Host to Network long */
uint32 htonl(uint32 value) { return ntohl(value); }
