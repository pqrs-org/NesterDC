/* KallistiOS 1.1.8

   assert.h
   (c)2002 Dan Potter

   assert.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#ifndef __ASSERT_H
#define __ASSERT_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/* This is nice and simple, modeled after the BSD one like most of KOS;
   the addition here is assert_msg(), which allows you to provide an
   error message. */
#define _assert(e) assert(e)

/* __FUNCTION__ is not ANSI, it's GCC, but we depend on GCC anyway.. */
#ifdef NDEBUG
#	define assert(e) ((void)0)
#	define assert_msg(e, m) ((void)0)
#else
#	define assert(e)        ((e) ? (void)0 : __assert(__FILE__, __LINE__, #e, NULL, __FUNCTION__))
#	define assert_msg(e, m) ((e) ? (void)0 : __assert(__FILE__, __LINE__, #e, m, __FUNCTION__))
#endif

/* Defined in assert.c */
void __assert(const char *file, int line, const char *expr,
	const char *msg, const char *func) __noreturn;

__END_DECLS

#endif	/* __ASSERT_H */

