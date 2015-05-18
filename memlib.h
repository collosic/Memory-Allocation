#ifndef MEMLIB
#define MEMLIB value

#include <stdlib.h>

void *mem_init(void);
void *mem_sbrk(int incr);
void mem_deinit(void);
void *getCurrentHeapSize();


/***************************************************
 * Wrappers for dynamic storage allocation functions
 ***************************************************/

void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size); 
void *Calloc(size_t nmemb, size_t size); 
void Free(void *ptr);


/*
 *  * Maximum heap size in bytes
 *   */
#define MAX_HEAP (20*(1<<20))  /* 20 MB */

/* error handling */
void unix_error(char *msg);


#endif
