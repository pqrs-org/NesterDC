#ifndef _KOS_SYS_PARAM_H
#define _KOS_SYS_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/types.h>

uint16 ntohs(uint16 value);
uint32 ntohl(uint32 value);
uint32 htons(uint32 value);
uint32 htonl(uint32 value);

#ifdef __cplusplus
}
#endif

#endif

