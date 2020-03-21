/* KallistiOS 1.1.8

   timer.c
   (c)2000-2001 Dan Potter
*/

#include <arch/arch.h>
#include <arch/timer.h>
#include <arch/irq.h>

CVSID("timer.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

/* Quick access macros */
#define TIMER8(o) ( *((volatile uint8*)(0xffd80000 + (o))) )
#define TIMER16(o) ( *((volatile uint16*)(0xffd80000 + (o))) )
#define TIMER32(o) ( *((volatile uint32*)(0xffd80000 + (o))) )
#define TOCR	0x00
#define TSTR	0x04
#define TCOR0	0x08
#define TCNT0	0x0c
#define TCR0	0x10
#define TCOR1	0x14
#define TCNT1	0x18
#define TCR1	0x1c
#define TCOR2	0x20
#define TCNT2	0x24
#define TCR2	0x28
#define TCPR2	0x2c

static int tcors[] = { TCOR0, TCOR1, TCOR2 };
static int tcnts[] = { TCNT0, TCNT1, TCNT2 };
static int tcrs[] = { TCR0, TCR1, TCR2 };

/* Pre-initialize a timer; set values but don't start it */
int timer_prime(int which, uint32 speed, int interrupts) {
	/* P0/64 scalar, maybe interrupts */
	if (interrupts)
		TIMER16(tcrs[which]) = 32 | 2;
	else
		TIMER16(tcrs[which]) = 2;
		
	/* Initialize counters; formula is P0/(tps*64) */
	TIMER32(tcnts[which]) = 50000000 / (speed*64);
	TIMER32(tcors[which]) = 50000000 / (speed*64);
	
	if (interrupts)
		timer_enable_ints(which);
	
	return 0;
}

/* Start a timer -- starts it running (and interrupts if applicable) */
int timer_start(int which) {
	TIMER8(TSTR) |= 1 << which;
	return 0;
}

/* Stop a timer -- and disables its interrupt */
int timer_stop(int which) {
	timer_disable_ints(which);
	
	/* Stop timer */
	TIMER8(TSTR) &= ~(1 << which);

	return 0;
}

/* Returns the count value of a timer */
uint32 timer_count(int which) {
	return TIMER32(tcnts[which]);
}

/* Clears the timer underflow bit and returns what its value was */
int timer_clear(int which) {
	uint16 value = TIMER16(tcrs[which]);
	TIMER16(tcrs[which]) &= ~0x100;
	
	return (value & 0x100) ? 1 : 0;
}

/* Spin-loop kernel sleep func: uses the secondary timer in the
   SH-4 to very accurately delay even when interrupts are disabled */
void timer_spin_sleep(int ms) {
	timer_prime(TMU1, 1000, 0);
	timer_clear(TMU1);
	timer_start(TMU1);

	while (ms > 0) {
		while (!(TIMER16(tcrs[TMU1]) & 0x100))
			;
		timer_clear(TMU1);
		ms--;
	}

	timer_stop(TMU1);
}



/* Enable timer interrupts (high priority); needs to move
   to irq.c sometime. */
void timer_enable_ints(int which) {
	volatile uint16 *ipra = (uint16*)0xffd00004;
	*ipra |= (0x000f << (12 - 4*which));
}

/* Disable timer interrupts; needs to move to irq.c sometime. */
void timer_disable_ints(int which) {
	volatile uint16 *ipra = (uint16*)0xffd00004;
	*ipra &= ~(0x000f << (12 - 4*which));
}

/* Check whether ints are enabled */
int timer_ints_enabled(int which) {
	volatile uint16 *ipra = (uint16*)0xffd00004;
	return (*ipra & (0x000f << (12 - 4*which))) != 0;
}

/* Millisecond timer */
static uint32 timer_ms_counter = 0;
static uint32 timer_ms_countdown;
static void timer_ms_handler(irq_t source, irq_context_t *context) {
	timer_ms_counter++;
}

void timer_ms_enable() {
	irq_set_handler(EXC_TMU2_TUNI2, timer_ms_handler);
	timer_prime(TMU2, 1, 1);
	timer_ms_countdown = timer_count(TMU2);
	timer_clear(TMU2);
	timer_start(TMU2);
}

void timer_ms_disable() {
	timer_stop(TMU2);
	timer_disable_ints(TMU2);
}

/* Return the number of ticks since KOS was booted */
void timer_ms_gettime(uint32 *secs, uint32 *msecs) {
	uint32 used;

	/* Seconds part comes from ms_counter */
	*secs = timer_ms_counter;

	/* Milliseconds, we check how much of the timer has elapsed */
	used = timer_ms_countdown - timer_count(TMU2);
	*msecs = (uint32)(used * 1000.0 / timer_ms_countdown);
}

/* Enable / Disable primary kernel timer */
void timer_primary_enable() {
	/* Preinit and enable timer 0 with interrupts, with HZ for jiffies */
	timer_prime(TMU0, HZ, 1);
	timer_clear(TMU0);
	timer_start(TMU0);
}

void timer_primary_disable() {
	timer_stop(TMU0);
	timer_disable_ints(TMU0);
}

/* Init */
int timer_init() {
	/* Disable all timers */
	TIMER8(TSTR) = 0;
	
	/* Set to internal clock source */
	TIMER8(TOCR) = 0;

	return 0;
}

/* Shutdown */
void timer_shutdown() {
	/* Disable all timers */
	TIMER8(TSTR) = 0;
}



