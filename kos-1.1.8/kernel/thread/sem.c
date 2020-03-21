/* KallistiOS 1.1.8

   sem.c
   (c)2001 Dan Potter
*/

/* Defines semaphores */

/**************************************/

#include <string.h>
#include <malloc.h>

#include <kos/thread.h>
#include <kos/limits.h>
#include <kos/sem.h>
#include <sys/queue.h>
#include <arch/syscall.h>
#include <arch/spinlock.h>

CVSID("sem.c,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp");

/**************************************/

/* Semaphore list spinlock */
static spinlock_t mutex;

/* Global list of semaphores */
static struct semlist sem_list;

/* Allocate a new semaphore; the semaphore will be assigned
   to the calling process and when that process dies, the semaphore
   will also die. */
semaphore_t *sem_create(int value) {
	semaphore_t	*sm;

	/* Create a semaphore structure */
	sm = (semaphore_t*)malloc(sizeof(semaphore_t));
	sm->owner = thd_current->tid;
	sm->count = value;
	TAILQ_INIT(&sm->blocked_wait);

	/* Add to the global list */
	spinlock_lock(&mutex);
	LIST_INSERT_HEAD(&sem_list, sm, g_list);
	spinlock_unlock(&mutex);

	/* Add to the process' list of semaphores */
	/* XXX Do later */

	return sm;
}

/* Take care of destroying a semaphore */
void sem_destroy(semaphore_t *sm) {
	/* XXX Do something with queued threads */

	/* Remove it from the global list */
	spinlock_lock(&mutex);
	LIST_REMOVE(sm, g_list);
	spinlock_unlock(&mutex);

	/* Free the memory */
	free(sm);
}

/* Wait on a semaphore */
static void sc_sem_wait(semaphore_t *sm) {
	/* If there's enough count left, then let the thread proceed */
	if (sm->count > 0) {
		sm->count--;
		return;
	}

	/* Otherwise, block the thread on semaphore wait and run another */
	thd_current->state = STATE_WAITSEM;
	TAILQ_INSERT_TAIL(&sm->blocked_wait, thd_current, thdq);
	thd_schedule();
}
void sem_wait(semaphore_t *sm) {
	assert (!irq_inside_int());
	if (irq_inside_int())
		return;
	SYSCALL(sc_sem_wait);
}

/* Wait on a semaphore, with timeout (in milliseconds) */
static void sc_sem_wait_timed(semaphore_t *sm, int timeout) {
	/* If there's enough count left, then let the thread proceed */
	if (sm->count > 0) {
		sm->count--;
		SET_MY_RETURN(0);
		return;
	}

	/* Otherwise, block the thread on semaphore wait and run another */
	thd_current->state = STATE_WAITSEM;
	thd_current->next_jiffy = jiffies + timeout * HZ / 1000;
	TAILQ_INSERT_TAIL(&sm->blocked_wait, thd_current, thdq);
	thd_schedule();
}
static void swt_ref(semaphore_t *v, int value) { }
int sem_wait_timed(semaphore_t *sem, int timeout) {
	/* Make sure we're not inside an interrupt */
	assert (!irq_inside_int());
	if (irq_inside_int()) return -1;

	/* Check for smarty clients */
	if (timeout <= 0) {
		sem_wait(sem);
		return 0;
	}


	/* Ok, call into "kernel space" to do the dirty work */
	swt_ref(sem, timeout);
	SYSCALL(sc_sem_wait_timed);
	/* implicit: return value is in place */
}

/* Signal a semaphore */ 
static void sc_sem_signal(semaphore_t *sm) {
	kthread_t	*thd;

	/* Is there anyone waiting? If so, pass off to them */
	if (!TAILQ_EMPTY(&sm->blocked_wait)) {
		/* Remove it from the queue */
		thd = TAILQ_FIRST(&sm->blocked_wait);
		TAILQ_REMOVE(&sm->blocked_wait, thd, thdq);

		/* Set a return value (if needed) */
		SET_RETURN(thd, 0);

		/* Re-activate it */
		thd->state = STATE_READY;
		thd_add_to_runnable(thd);
	} else {
		/* No one is waiting, so just add another tick */
		sm->count++;
	}
}
void sem_signal(semaphore_t *sm) {
	if (irq_inside_int()) {
		sc_sem_signal(sm);
	} else {
		SYSCALL(sc_sem_signal);
	}
}

/* Return the semaphore count */
int sem_count(semaphore_t *sm) {
	/* Look for the semaphore */
	return sm->count;
}

/* Free all semaphores for the given process' pid */
/* void sem_freeall(pid_t pid) {
	semaphore_t *n1, *n2;

	n1 = LIST_FIRST(&sem_list);
	while (n1 != NULL) {
		n2 = LIST_NEXT(n1, g_list);
		if (n1->owner == pid) {
			LIST_REMOVE(n1, g_list);
			sem_destroy(n1);
		}
		n1 = n2;
	}
} */

/* Called by the periodic thread interrupt to look for timed out
   sem_wait_timed calls */
void sem_check_timeouts() {
	semaphore_t	*sm;
	kthread_t	*thd, *tn;
	
	/* Go through the list and look for timed-out waiting threads */
	LIST_FOREACH(sm, &sem_list, g_list) {
		/* Go through the queue of waiting threads on this semaphore */
		thd = TAILQ_FIRST(&sm->blocked_wait);
		while (thd != NULL) {
			tn = TAILQ_NEXT(thd, thdq);
			if (thd->next_jiffy && jiffies >= thd->next_jiffy) {
				/* Set an error code */
				SET_RETURN(thd, -1);

				/* Remove it from the blocked queue */
				TAILQ_REMOVE(&sm->blocked_wait, thd, thdq);
				
				/* Re-activate it */
				thd->state = STATE_READY;
				thd->next_jiffy = 0;
				thd_add_to_runnable(thd);
			}
			thd = tn;
		}
	}
}
      
/* Initialize semaphore structures */
int sem_init() {
	LIST_INIT(&sem_list);
	return 0;
}

/* Shut down semaphore structures */
void sem_shutdown() { }


