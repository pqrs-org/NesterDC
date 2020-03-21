/* KallistiOS 1.1.8

   arch/gba/arch.h
   (c)2001 Dan Potter
   
   arch.h,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp
*/

#ifndef __ARCH_ARCH_H
#define __ARCH_ARCH_H

/* Number of timer ticks per second (if using threads) */
#define HZ		100

/* Default thread stack size (if using threads) */
#define THD_STACK_SIZE	8192

/* Default video mode */

/* Default serial parameters */

/* Panic function */
void panic(char *str);

/* Prototype for the portable kernel main() */
int kernel_main(const char *args);

/* Kernel C-level entry point */
int arch_main();

/* Kernel "quit" point */
void arch_exit();

/* Kernel "reboot" call */
void arch_reboot();

#endif	/* __ARCH_ARCH_H */

