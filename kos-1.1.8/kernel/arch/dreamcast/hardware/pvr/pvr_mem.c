/* KallistiOS 1.1.8

   pvr_mem.c
   (C)2002 Dan Potter

 */

#include <assert.h>
#include <dc/pvr.h>
#include "pvr_internal.h"

#include <malloc.h>	/* For the struct mallinfo defs */

/*

This module basically serves as a KOS-friendly front end and support routines
for the pvr_mem_core module, which is a dlmalloc-derived malloc for use with
the PVR memory pool.

I was originally going to make a totally seperate thing that could be used
to generically manage any memory pool, but then I realized what a gruelling
and thankless task that would be when starting with dlmalloc, so we have this
instead. ^_^;
 
*/

CVSID("pvr_mem.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Bring in some prototypes from pvr_mem_core.c */
/* We can't directly include its header because of name clashes with
   the real malloc header */
extern void * pvr_int_malloc(size_t bytes);
extern void pvr_int_free(void *ptr);
extern struct mallinfo pvr_int_mallinfo();
extern void pvr_int_mem_reset();


/* PVR RAM base; NULL is considered invalid */
static pvr_ptr_t pvr_mem_base = NULL;
#define CHECK_MEM_BASE assert_msg(pvr_mem_base != NULL, \
	"pvr_mem_* used, but PVR hasn't been initialized yet")

/* Used in pvr_mem_core.c */
void * pvr_int_sbrk(size_t amt) {
	uint32 old, n;
	
	/* Are we valid? */
	CHECK_MEM_BASE;

	/* Try to increment it */
	old = (uint32)pvr_mem_base;
	n = old + amt;

	/* Did we run over? */
	if (n > PVR_RAM_INT_TOP)
		return (void *)-1;

	/* Nope, everything's cool */
	pvr_mem_base = (pvr_ptr_t)n;
	return (pvr_ptr_t)old;
}

/* Allocate a chunk of memory from texture space; the returned value
   will be relative to the base of texture memory (zero-based) */
pvr_ptr_t pvr_mem_malloc(size_t size) {
	uint32 rv32;
	
	CHECK_MEM_BASE;

	rv32 = (uint32)pvr_int_malloc(size);
	assert_msg( (rv32 & 0x1f) == 0,
		"dlmalloc's alignment is broken; please make a bug report");
	return (pvr_ptr_t)rv32;
}

/* Free a previously allocated chunk of memory */
void pvr_mem_free(pvr_ptr_t chunk) {
	CHECK_MEM_BASE;
		
	pvr_int_free((void *)chunk);
}

/* Return the number of bytes available still in the memory pool */
static uint32 pvr_mem_available_int() {
	struct mallinfo mi = pvr_int_mallinfo();

	/* This magic formula is modeled after mstats() */
	return mi.arena - mi.uordblks;
}
uint32 pvr_mem_available() {
	CHECK_MEM_BASE;

	return pvr_mem_available_int();
}

/* Reset the memory pool, equivalent to freeing all textures currently
   residing in RAM. This _must_ be done on a mode change, configuration
   change, etc. */
void pvr_mem_reset() {
	if (!pvr_state.valid)
		pvr_mem_base = NULL;
	else {
		pvr_mem_base = (pvr_ptr_t)(PVR_RAM_INT_BASE + pvr_state.texture_base);
		pvr_int_mem_reset();
	}
}


