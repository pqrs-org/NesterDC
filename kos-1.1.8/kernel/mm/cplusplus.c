/* KallistiOS 1.1.8

   cplusplus.c
   (c)2002 Dan Potter

   This is just a wrapper around malloc() and free() for C++ programs.
 */

#include <malloc.h>

CVSID("cplusplus.c,v 1.1.1.1 2002/10/09 13:59:14 tekezo Exp");

void *__builtin_new(int size) {
	return malloc(size);
}

void *__builtin_vec_new(int size) {
	return __builtin_new(size);
}

void __builtin_delete(void *ptr) {
	free(ptr);
}

void __builtin_vec_delete(void *ptr) {
	__builtin_delete(ptr);
}

