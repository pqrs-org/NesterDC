/* KallistiOS 1.1.8

   rand.c
   (c)2000-2001 Dan Potter

   Slightly less than optimal random number function
*/

#include <stdlib.h>

CVSID("rand.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

static unsigned long seed=123;
#define RNDC 1013904223
#define RNDM 1164525

void srand(unsigned int s) {
	seed = s;
}

int rand() {
	seed = seed * RNDM + RNDC;
	return seed & RAND_MAX;
}

