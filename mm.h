#ifndef MM
#define MM value

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define FALSE 0
#define TRUE 1

/* $begin mallocinterface */
int mm_init(void); 
void *mm_malloc(size_t size); 
void mm_free(void *bp);
/* $end mallocinterface */

void mm_checkheap(int verbose);
void *mm_realloc(void *ptr, size_t size);

/* Unused. Just to keep us compatible with the 15-213 malloc driver */
typedef struct {
    char *team;
    char *name1, *email1;
    char *name2, *email2;
} team_t;

extern team_t team;

/* privat global for setting placement algorightm */
static int isFirstFit;

/* Our internal helper functions */
int evaluate(char *cmdline);
int getCommandType(char *cmd);
int parseline(char *buf, char **argv);
void* firstFit(size_t asize);
void* bestFit(size_t asize);

/* inline function declartions */
void setBestFit() { isFirstFit = FALSE; };
void setFirstFit() { isFirstFit = TRUE; };


/* enum for evaluating a command given at the prompt */
enum commands { ALLOCATE = 0
                , FREE = 1
                , BLOCKLIST = 2
                , WRITEHEAP = 3
                , PRINTHEAP = 4
                , BESTFIT = 5
                , FIRSTFIT = 6
                , QUIT = 7 };
#endif