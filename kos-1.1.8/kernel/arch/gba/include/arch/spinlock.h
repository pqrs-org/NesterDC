/* KallistiOS 1.1.8

   arch/gba/include/spinlock.h
   (c)2001 Dan Potter
   
   spinlock.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#ifndef __ARCH_SPINLOCK_H
#define __ARCH_SPINLOCK_H

/* Defines processor specific spinlocks; if you include this file, you
   must first include kos/thread.h. */

/* Spinlock data type */
/* typedef int spinlock_t; */
typedef char spinlock_t;

/* Value initializer */
#define SPINLOCK_INITIALIZER 0

/* Initialize a spinlock */
#define spinlock_init(A) *(A) = SPINLOCK_INITIALIZER

/* Spin on a lock */
/* #define spinlock_lock(A) do { \
		if (thd_enabled) { \
			if (*(A)) { \
				while (*(A)) \
					; \
				*(A) = 1; \
			} \
		} \
	} while (0) */
#define spinlock_lock(A) do { } while (0)

/* Free a lock */
/* #define spinlock_unlock(A) do { \
		if (thd_enabled) { \
			*(A) = 0; \
		} \
	} while (0) */
#define spinlock_unlock(A) do { } while (0)


#endif	/* __ARCH_SPINLOCK_H */

