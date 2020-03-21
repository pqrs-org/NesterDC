/* KallistiOS 1.1.8

   cond.c
   (c)2001 Dan Potter
*/

/* Defines condition variables, which are like semaphores that automatically
   signal all waiting processes when a signal() is called. */

#include <string.h>
#include <malloc.h>

#include <kos/thread.h>
#include <kos/limits.h>
#include <kos/cond.h>
#include <sys/queue.h>
#include <arch/syscall.h>
#include <arch/spinlock.h>

CVSID("cond.c,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp");

/**************************************/

/* Condvar list spinlock */
static spinlock_t mutex;

/* Global list of condvars */
static struct condlist cond_list;

/* Allocate a new condvar; the condvar will be assigned
   to the calling process and when that process dies, the condvar
   will also die. */
condvar_t *cond_create() {
	condvar_t	*cv;

	/* Create a condvar structure */
	cv = (condvar_t*)malloc(sizeof(condvar_t));
	cv->owner = thd_current->tid;
	TAILQ_INIT(&cv->blocked_wait);

	/* Add to the global list */
	spinlock_lock(&mutex);
	LIST_INSERT_HEAD(&cond_list, cv, g_list);
	spinlock_unlock(&mutex);

	return cv;
}

/* Free a condvar */
void cond_destroy(condvar_t *cv) {
	/* XXX Do something with queued threads */

	/* Remove it from the global list */
	spinlock_lock(&mutex);
	LIST_REMOVE(cv, g_list);
	spinlock_unlock(&mutex);

	/* Free the memory */
	free(cv);
}

/* Wait on a condvar */
static void sc_cond_wait(condvar_t *cv) {
	/* Block the thread on condition wait and run another */
	thd_current->state = STATE_WAITCOND;
	TAILQ_INSERT_TAIL(&cv->blocked_wait, thd_current, thdq);
	thd_schedule();
}
void cond_wait(condvar_t *cv) {
	if (irq_inside_int()) return;
	SYSCALL(sc_cond_wait);
}

/* Signal a condvar */ 
static void sc_cond_signal(condvar_t *cv) {
	kthread_t	*thd, *n;

	/* Is there anyone waiting? If so, pass off to them */
	thd = TAILQ_FIRST(&cv->blocked_wait);
	while (thd != NULL) {
		/* Grab the next thread */
		n = TAILQ_NEXT(thd, thdq);
		
		/* Re-activate it */
		thd->state = STATE_READY;
		thd_add_to_runnable(thd);

		/* Advance */
		thd = n;
	}
}
void cond_signal(condvar_t *cv) {
	if (irq_inside_int()) {
		sc_cond_signal(cv);
	} else {
		SYSCALL(sc_cond_signal);
	}
}

/* Free all condvars for the given process' pid */
/* void cond_freeall(pid_t pid) {
	condvar_t *n1, *n2;

	n1 = LIST_FIRST(&cond_list);
	while (n1 != NULL) {
		n2 = LIST_NEXT(n1, g_list);
		if (n1->owner == pid) {
			LIST_REMOVE(n1, g_list);
			cond_destroy(n1);
		}
		n1 = n2;
	}
} */

/* Initialize condvar structures */
int cond_init() {
	LIST_INIT(&cond_list);
	return 0;
}

/* Shut down condvar structures */
void cond_shutdown() { }
