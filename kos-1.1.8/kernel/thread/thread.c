/* KallistiOS 1.1.8

   kernel/thread.c
   (c)2000-2002 Dan Potter
*/

#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <kos/thread.h>
#include <kos/sem.h>
#include <kos/cond.h>
#include <arch/irq.h>
#include <arch/timer.h>
#include <arch/arch.h>
#include <arch/syscall.h>

CVSID("thread.c,v 1.1.1.1 2002/10/09 13:59:15 tekezo Exp");

/*

This module supports thread scheduling in KOS. The timer interrupt is used
to re-schedule the processor HZ times per second in pre-emptive mode. 
This is a fairly simplistic scheduler, though it does employ some 
standard advanced OS tactics like priority scheduling and semaphores.

Some of this code ought to look familiar to BSD-heads; I studied the
BSD kernel quite a bit to get some ideas on priorities, and I am
also using their queue library verbatim (sys/queue.h).
 
*/

/*****************************************************************************/
/* Thread scheduler data */

/* Thread list. This includes all threads except dead ones. */
struct ktlist thd_list;

/* Run queue. This is more like on a standard time sharing system than the
   previous versions. The top element of this priority queue should be the
   thread that is ready to run next. When a thread is scheduled, it will be
   removed from this queue. When it's de-scheduled, it will be re-inserted
   by its priority value at the end of its priority group. Note that right
   now this condition is being broken because sleeping threads are on the
   same queue. We deal with those in thd_switch below. */
struct ktqueue run_queue;

/* "Jiffy" count: this is basically a counter that gets incremented each
   time a timer interrupt happens. */
vuint32 jiffies;

/* The currently executing thread */
kthread_t *thd_current = NULL;

/* Thread mode */
int thd_mode = THD_MODE_COOP;

/*****************************************************************************/
/* Debug */
#include <arch/dbgio.h>

static const char *state_to_str(int state) {
	switch(state) {
		case STATE_ZOMBIE:
			return "ZOMBIE    ";
		case STATE_RUNNING:
			return "RUNNING   ";
		case STATE_READY:
			return "READY     ";
		case STATE_SLEEP:
			return "SLEEP     ";
		case STATE_WAITSEND:
			return "WAITSEND  ";
		case STATE_WAITRECV:
			return "WAITRECV  ";
		case STATE_WAITREPLY:
			return "WAITREPLY ";
		case STATE_WAITSEM:
			return "WAITSEM   ";
		case STATE_WAITCOND:
			return "WAITCOND  ";
		default:
			return "UNKNOWN   ";
	}
}

int thd_pslist() {
	kthread_t *cur;

	printf("All threads:\n");
	printf("addr\t\ttid\tprio\tflags\tnext_jiffy\tstate     name\n");
	
	LIST_FOREACH(cur, &thd_list, t_list) {
		printf("%08x\t", CONTEXT_PC(cur->context));
		printf("%d\t", cur->tid);
		if (cur->prio == PRIO_MAX)
			printf("MAX\t");
		else
			printf("%d\t", cur->prio);
		printf("%x\t", cur->flags);
		printf("%d\t\t", cur->next_jiffy);
		printf("%s", state_to_str(cur->state));
		printf("%s\n", cur->label);
	}
	printf("--end of list--\n");

	return 0;
}

int thd_pslist_queue() {
	kthread_t *cur;

	printf("Queued threads:\n");
	printf("addr\t\ttid\tprio\tflags\tnext_jiffy\tstate     name\n");
	TAILQ_FOREACH(cur, &run_queue, thdq) {
		printf("%08x\t", CONTEXT_PC(cur->context));
		printf("%d\t", cur->tid);
		if (cur->prio == PRIO_MAX)
			printf("MAX\t");
		else
			printf("%d\t", cur->prio);
		printf("%x\t", cur->flags);
		printf("%d\t\t", cur->next_jiffy);
		printf("%s", state_to_str(cur->state));
		printf("%s\n", cur->label);
	}
	printf("--end of list--\n\n");

	return 0;
}

/*****************************************************************************/
/* Returns a fresh thread ID for each new thread */

/* Highest thread id (used when assigning next thread id) */
static tid_t tid_highest;


/* Return the next available thread id (assumes wraparound will not run
   into old processes). */
static tid_t thd_next_free() {
	int id;
	id = tid_highest++;
	return id;
}

/* Given a thread ID, locates the thread structure */
kthread_t *thd_by_tid(tid_t tid) {
	kthread_t *np;

	LIST_FOREACH(np, &thd_list, t_list) {
		if (np->tid == tid)
			return np;
	}

	return NULL;
}


/*****************************************************************************/
/* Thread support routines: idle task and start task wrapper */

/* An idle function. This function literally does nothing but loop
   forever. It's meant to be used for an idle task. */
static void thd_idle_task(void *param) {
	/* Uncomment these if you want some debug for deadlocking */
/*	int old = irq_disable();
#ifndef NDEBUG
	thd_pslist();
	printf("Inside idle task now\n");
#endif
	irq_restore(old); */
	for (;;) {
		arch_sleep();	/* We can safely enter sleep mode here */
	}
}

/* Thread execution wrapper; when the thd_create function below
   adds a new thread to the thread chain, this function is the one
   that gets called in the new context. */
void thd_birth(void (*routine)(void *param), void *param) {
	/* Call the thread function */
	routine(param);

	/* Call Dr. Kevorkian; after this executes we could be killed
	   at any time. */
	thd_current->state = STATE_ZOMBIE;

	/* Wait for death */
	for (;;) {
		arch_sleep();	/* We can safely enter sleep mode here */
	}
}

/* An exit syscall for user-mode threads */
void thd_exit() {
	thd_current->state = STATE_ZOMBIE;
	thd_schedule();
}


/*****************************************************************************/
/* Thread creation and deletion */

/* Enqueue a process in the runnable queue; adds it right after the
   process group of the same priority. */
void thd_add_to_runnable(kthread_t *t) {
	kthread_t	*i;
	int		done;

	if (t->flags & THD_QUEUED)
		return;

	done = 0;

	/* Look for a thread of lower priority and insert
	   before it. If there is nothing on the run queue, we'll 
	   fall through to the bottom. */
	TAILQ_FOREACH(i, &run_queue, thdq) {
		if (i->prio > t->prio) {
			TAILQ_INSERT_BEFORE(i, t, thdq);
			done = 1;
			break;
		}
	}

	/* Didn't find one, put it at the end */
	if (!done)
		TAILQ_INSERT_TAIL(&run_queue, t, thdq);

	t->flags |= THD_QUEUED;
}

/* Removes a thread from the runnable queue, if it's there. */
int thd_remove_from_runnable(kthread_t *thd) {
	if (!(thd->flags & THD_QUEUED)) return 0;
	thd->flags &= ~THD_QUEUED;
	TAILQ_REMOVE(&run_queue, thd, thdq);
	return 0;
}

/* New thread function; given a routine address, it will create a
   new kernel thread with a default stack. When the routine
   returns, the thread will exit. Returns the new thread struct. */
kthread_t *thd_create(void (*routine)(void *param), void *param) {
	kthread_t	*nt;
	tid_t		tid;
	uint32		params[2];
	int		oldirq = 0;

	nt = NULL;

	if (!irq_inside_int()) {
		oldirq = irq_disable();
	}

	/* Get a new thread id */
	tid = thd_next_free();
	if (tid < 0) goto exit;

	/* Create a new thread structure */
	nt = malloc(sizeof(kthread_t));
	if (nt == NULL) goto exit;

	/* Create a new thread stack */
	nt->stack = (uint32*)malloc(THD_STACK_SIZE);
	nt->stack_size = THD_STACK_SIZE;
	
	/* Populate the context */
	params[0] = (uint32)routine;
	params[1] = (uint32)param;
	irq_create_context(&nt->context,
		((uint32)nt->stack)+nt->stack_size,
		(uint32)thd_birth, params, 0);

	nt->tid = tid;
	nt->prio = PRIO_DEFAULT;
	nt->flags = THD_DEFAULTS;
	nt->state = STATE_READY;
	nt->next_jiffy = 0;
	strcpy(nt->label, "[un-named kernel thread]");
	strcpy(nt->pwd, thd_current->pwd);

	/* Insert it into the thread list */
	LIST_INSERT_HEAD(&thd_list, nt, t_list);

	/* Schedule it */
	thd_add_to_runnable(nt);

exit:
	if (!irq_inside_int())
		irq_restore(oldirq);
	return nt;
}

/* Given a thread id, this function removes the thread from
   the execution chain. */
int thd_destroy(kthread_t *thd) {
	int oldirq = 0;
	
	if (thd == thd_current) {
		thd->state = STATE_ZOMBIE;
		return 0;
	}

	if (!irq_inside_int())
		oldirq = irq_disable();

	/* De-schedule the thread if it's scheduled and free the
	   thread structure */
	thd_remove_from_runnable(thd);
	LIST_REMOVE(thd, t_list);

	/* Free its stack */
	free(thd->stack);

	/* Free the thread */
	free(thd);

	if (!irq_inside_int())
		irq_restore(oldirq);

	return 0;
}

/*****************************************************************************/
/* Thread attribute functions */

/* Set a thread's priority */
int thd_set_prio(kthread_t *thd, prio_t prio) {
	/* Set the new priority */
	thd->prio = prio;
	return 0;
}

/*****************************************************************************/
/* Scheduling routines */

/* Thread scheduler; this function will find a new thread to run when a 
   context switch is requested. No work is done in here except to change
   out the thd_current variable contents. Assumed that we are in an 
   interrupt context. */
void thd_schedule() {
	int		dontenq, count;
	kthread_t	*thd, *tnext;

	/* Destroy all threads marked STATE_ZOMBIE */
	dontenq = 0;
	thd = LIST_FIRST(&thd_list);
	count = 0;
	while (thd != NULL) {
		tnext = LIST_NEXT(thd, t_list);

		if (thd->state == STATE_ZOMBIE) {
			if (thd == thd_current)
				dontenq = 1;
			thd_destroy(thd);
		}

		thd = tnext;
		count++;
	}

	/* If there's only one thread left, it's the idle task: exit the OS */
	if (count == 1) {
		dbgio_printf("\nthd_schedule: idle task is the only thing left; exiting\n");
		arch_exit();
	}

	/* Re-queue the last "current" thread onto the run queue if
	   it didn't die */
	if (!dontenq && thd_current->state == STATE_RUNNING) {
		thd_current->state = STATE_READY;
		thd_add_to_runnable(thd_current);
	}

	/* Search first for threads in STATE_SLEEP that have timed out; add them
	   to the run queue again. */
	LIST_FOREACH(thd, &thd_list, t_list) {
		if (thd->state == STATE_SLEEP
				&& jiffies >= thd->next_jiffy) {
			thd->state = STATE_READY;
			thd->next_jiffy = 0;
			thd_add_to_runnable(thd);
		}
	}

	/* Look for timed out semaphore waits */
	sem_check_timeouts();

	/* Search downwards through the run queue for a runnable thread; if
	   we don't find a normal runnable thread, the idle process will
	   always be there at the bottom. */
	TAILQ_FOREACH(thd, &run_queue, thdq) {
		/* dbgio_printf("checking thread %08x (%d)\n", thd, thd->tid); */
		/* Is it runnable? If not, keep going */
		if (thd->state == STATE_READY)
			break;
	}
	/* dbgio_printf("\n"); */

	/* Check for waking sleeper threads */
	/* Do later */

	/* Didn't find one? Big problem here... */
	if (thd == NULL) {
		thd_pslist();
		panic("couldn't find a runnable thread");
	}
		
	/* We should now have a runnable thread, so remove it from the
	   run queue and switch to it. */
	thd_remove_from_runnable(thd);

	thd_current = thd;
	thd->next_jiffy = 0;
	thd->state = STATE_RUNNING;

	/* Make sure the thread hasn't underrun its stack */
	assert_msg( CONTEXT_SP(thd_current->context) >= (ptr_t)(thd_current->stack), "Thread stack underrun" );

	irq_set_context(&thd_current->context);
}

/* Temporary priority boosting function: call this from within an interrupt
   to boost the given thread to the front of the queue. This will cause the
   interrupt return to jump back to the new thread instead of the one that
   was executing (unless it was already executing). */
void thd_schedule_next(kthread_t *thd) {
	/* Unfortunately we have to take care of this here */
	if (thd_current->state == STATE_ZOMBIE) {
		thd_destroy(thd);
	} else if (thd_current->state == STATE_RUNNING) {
		thd_current->state = STATE_READY;
		thd_add_to_runnable(thd_current);
	}

	thd_current = thd;
	thd->next_jiffy = 0;
	thd->state = STATE_RUNNING;
	irq_set_context(&thd_current->context);
}


/* Timer function; just calls thd_schedule and swaps out irq context. */
void thd_timer_hnd(irq_t src, irq_context_t *context) {
	/* if (!(jiffies % 10)) */
		thd_schedule();
	jiffies++;
}

/* Thread blocking based sleeping; this is the preferred way to
   sleep because it eases the load on the system for the other
   threads. */
void thd_sleep_jiffies(int jif) {
	assert_msg( thd_mode == THD_MODE_PREEMPT, "thd_sleep_jiffies() doesn't work in co-op mode right now" );
	thd_current->next_jiffy = jiffies + jif;
	thd_current->state = STATE_SLEEP;
	thd_pass();
}

void thd_sleep(int ms) {
	if (thd_mode == THD_MODE_PREEMPT)
		thd_sleep_jiffies(ms * HZ / 1000);
	else {
		timer_spin_sleep(ms);
	}
}

/* Thread pass syscall */
static void sc_thd_pass() {
	thd_schedule();
}

/* Manually cause a re-schedule */
void thd_pass() {
	/* Makes no sense inside int */
	if (irq_inside_int()) return;

	SYSCALL(sc_thd_pass);
}

/*****************************************************************************/
/* Retrive / set thread label */                     
const char *thd_get_label(kthread_t *thd) {
	return thd->label;
}

void thd_set_label(kthread_t *thd, const char *label) {
	strncpy(thd->label, label, sizeof(thd->label) - 1);
}

/* Find the current thread */
kthread_t *thd_get_current() {
	return thd_current;
}

/* Retrieve / set thread pwd */   
const char *thd_get_pwd(kthread_t *thd) {
	return thd->pwd;
}

void thd_set_pwd(kthread_t *thd, const char *pwd) {
	strncpy(thd->pwd, pwd, sizeof(thd->pwd) - 1);
}

/*****************************************************************************/

/* Change threading modes */
int thd_set_mode(int mode) {
	/* Nothing to change? */
	if (thd_mode == mode)
		return thd_mode;

	if (thd_mode == THD_MODE_COOP) {
		/* Switch to pre-emptive mode */
		irq_set_handler(TIMER_IRQ, thd_timer_hnd);
		timer_primary_enable();
	} else {
		/* Switch to co-op mode */
		timer_primary_disable();
		irq_set_handler(TIMER_IRQ, NULL);
	}

	return thd_mode;
}

/*****************************************************************************/
/* Init/shutdown */

/* Init */
int thd_init(int mode) {
	kthread_t *idle, *kern;

	/* Setup our mode as appropriate */
	thd_mode = mode;

	/* Initialize handle counters */
	tid_highest = 1;

	/* Initialize the thread list */
	LIST_INIT(&thd_list);

	/* Initialize the run queue */
	TAILQ_INIT(&run_queue);

	/* Setup a kernel task for the currently running "main" thread */
	kern = thd_create(NULL, NULL);
	strcpy(kern->label, "[kernel]");
	kern->state = STATE_RUNNING;

	/* De-scehdule the thread (it's STATE_RUNNING) */
	thd_remove_from_runnable(kern);
	
	/* Setup an idle task that is always ready to run, in case everyone
	   else is blocked on something. */
	idle = thd_create(thd_idle_task, NULL);
	strcpy(idle->label, "[idle]");
	thd_set_prio(idle, PRIO_MAX);
	idle->state = STATE_READY;

	/* Main thread -- the kern thread */
	thd_current = kern;
	irq_set_context(&kern->context);

	/* Re-initialize jiffy counter */
	jiffies = 0;

	/* Initialize thread sync primitives */
	sem_init();
	cond_init();

	/* If we're in pre-emptive mode, then setup the timer scheduler */
	if (thd_mode == THD_MODE_PREEMPT) {
		irq_set_handler(TIMER_IRQ, thd_timer_hnd);
		timer_primary_enable();
	}

	return 0;
}

/* Shutdown */
void thd_shutdown() {
	kthread_t *n1, *n2;

	/* Disable pre-emption, if neccessary */
	if (thd_mode == THD_MODE_PREEMPT) {
		timer_primary_disable();
		irq_set_handler(TIMER_IRQ, NULL);
	}

	/* Kill remaining live threads */
	n1 = LIST_FIRST(&thd_list);
	while (n1 != NULL) {
		n2 = LIST_NEXT(n1, t_list);
		free(n1->stack);
		free(n1);
		n1 = n2;
	}

	/* Shutdown thread sync primitives */
	sem_shutdown();
	cond_shutdown();
}


