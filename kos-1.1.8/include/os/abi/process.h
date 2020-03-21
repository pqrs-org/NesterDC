/* KallistiOS 1.1.8

   os/abi/process.h
   (c)2000-2001 Jordan DeLong

   process.h,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp

*/

#ifndef __OS_ABI_PROCESS_H
#define __OS_ABI_PROCESS_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <os/process.h>
#include <os/elf.h>

typedef struct {
	service_abi_t	hdr;	/* header info */

	elf_prog_t*	(*load)(const char *fn);
	int		(*load_and_exec)(const char *fn, int argc, char **argv);
	kthread_t*	(*load_and_fork)(const char *fn, int argc, char **argv);
} abi_process_t;

__END_DECLS

#endif	/* __OS_ABI_PROCESS_H */

