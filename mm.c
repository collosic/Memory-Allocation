/* 
 * Simple, 32-bit and 64-bit clean allocator based on implicit free
 * lists, first fit placement, and boundary tag coalescing, as described
 * in the CS:APP2e text. Blocks must be aligned to doubleword (8 byte) 
 * boundaries. Minimum block size is 16 bytes. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"
#include "map.h"

/*
 * If NEXT_FIT defined use next fit search, else use first fit search 
 */
//#define NEXT_FIT


/* Here we define the size of our alignment from the default of 8 bytes to an adjusted
 * 4 byte WORD.  If WORD_ALIGNMENT is defined then the alignment will be changed from DSIZE (8 bytes)
 * to a WSIZE (4 bytes) alignment. */
#define WORD_ALIGNMENT

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ //line:vm:mm:beginconst
#define DSIZE       8       /* Doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  //line:vm:mm:endconst 

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            //line:vm:mm:get
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    //line:vm:mm:put

/* Read the size and allocated fields from address p */
#ifdef WORD_ALIGNMENT
#define GET_SIZE(p)  (GET(p) & ~0x3)                   //line:vm:mm:getsize
#else 
#define GET_SIZE(p)  (GET(p) & ~0x7)                   //line:vm:mm:getsize
#endif
#define GET_ALLOC(p) (GET(p) & 0x1)                    //line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      //line:vm:mm:hdrp
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) //line:vm:mm:nextblkp
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) //line:vm:mm:prevblkp
/* $end mallocmacros */

/* Global variables */
int allocate_counter = 0;
node_t *blocklist;
static char *heap_listp = 0;  /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover;           /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);
void clean_up(node_t *blocklist, void *heap);


int main(int argc, char *argv[])
{
    blocklist = malloc(sizeof(node_t));
    int program_is_running = 1;
    char cmdline[MAXLINE];

    /* first thing we need to do is to initialize the memory and heap */
    mm_init();

    while (program_is_running) {
        printf("> ");
        fflush(stdout);
        fgets(cmdline, MAXLINE, stdin);

        if (feof(stdin)) {
            exit(0);
        }
        /* Evaluate */
        program_is_running = evaluate(cmdline);
        print_list(blocklist);
    }
	free_list(blocklist);
	blocklist=NULL;
    return 0;
}


int evaluate(char *cmdline) {
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */

    /* READ THIS NOTE
     * After parsline has completed your command will resid in 
     * argv[0] and your arguments in the following elements.  
     * parseline() will return the number of arguments passed in by
     * the user, use it as you please and remember that *argv[] ends
     * with a NULL.  I leave each function to verify it's args are correct
     */
    strcpy(buf, cmdline);
    parseline(buf, argv);
    
    // now determine what command we will use
    int type = getCommandType(argv[0]);

    // now call the function that is needed
    switch (type) {
        /* Here is an example on how you can use this switch statement
         * printheap(argc, argv);
         */
        case ALLOCATE:      printf("%i\n", allocate(argv));
                            break;
        case FREE:          free_block(argv);
                            break;
        case BLOCKLIST:     print_blocklist();
                            break;
        case WRITEHEAP:     write_heap(argv);
                            break;
        case PRINTHEAP:     // call your function here
                            break;
        case BESTFIT:       setBestFit();
                            break;
        case FIRSTFIT:      setFirstFit();
                            break;
        case QUIT:          mem_deinit(); 
                            return 0;
        default:            // This means invalid command
                            puts("invalid command entered");
    }
    return 1;
}

int getCommandType(char *cmd) {
    if (cmd == NULL) {
        return -1;
    } else if (!strcmp(cmd, "allocate")) {
        return ALLOCATE;
    } else if (!strcmp(cmd, "free")) {
        return FREE;
    } else if (!strcmp(cmd, "blocklist")) {
        return BLOCKLIST;
    } else if (!strcmp(cmd, "writeheap")) {
        return WRITEHEAP;
    } else if (!strcmp(cmd, "printheap")) {
        return PRINTHEAP;
    } else if (!strcmp(cmd, "bestfit")) {
        return BESTFIT;
    } else if (!strcmp(cmd, "firstfit")) {
        return FIRSTFIT;
    } else if (!strcmp(cmd, "quit")) {
        return QUIT;
    } else {
        return -1;
    }
}



//A: allocate fx "wrapper"
int allocate (char *argv[]) {
    unsigned int amount;
    if(sscanf(argv[1], "%u", &amount) != 1) {
        printf("what did you put in that cmd line? not an int!\n");
        return -1;
    }
    printf("Hello we are in allocate!\n");
    char * p = mm_malloc(amount);
    if(p != NULL){
        printf("this is allocated ptr bp: %p\n", p);		
        allocate_counter = allocate_counter + 1;
        return insert_node(blocklist, allocate_counter, p);
    } else {
        printf("We have null pointer on our hands, run for cover\n");
        return 0;
    }

}

//A: free "wrapper"
void free_block(char *argv[]) {
    int block_num;
    if(sscanf(argv[1], "%i", &block_num) != 1) {
        printf("what did you put in that cmd line? not an int!\n");
    }
    printf("Hello from free fx\n");
    //here we get that pointer from our LL via void *get_addy(int block_num) fx or something
    mm_free(find_node(blocklist, block_num));
	remove_by_index(blocklist, block_num);
	
    
}

//A: blocklist fx
void print_blocklist() {
	char *bp = heap_listp;
	printf("Size\tAllocated\tStart\tEnd\n");
	//skipping prologue here
	for (bp = heap_listp+2*WSIZE; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
		 printblock(bp);
	}

}


void write_heap(char *argv[]) {
    int block_num, repeats;
    char character;
    if(sscanf(argv[1], "%i", &block_num) != 1) {
        printf("what did you put in that cmd line? not an int!\n");
        return;
    }
    if(sscanf(argv[2], "%c", &character) != 1) {
        printf("what did you put in that cmd line? not an int!\n");
        return;
    }
    if(sscanf(argv[3], "%i", &repeats) != 1) {
        printf("what did you put in that cmd line? not an int!\n");
        return;
    }

    /* Let's get the block pointer from our list */
    char *bp = find_node(blocklist, block_num);
    int i = 0;
    for (;i < repeats; i++){
        *(bp+i) = character;
    }
    *(bp+i) = '\0';
}


int parseline(char *buf, char **argv)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    return argc;
}


/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void)
{
    /* this function will initialize our entire memory space that the heap
     * will live in.  Think of this as our VM or even our DRAM
     */
    mem_init();

    /* initialize the placement algo */
    setFirstFit();

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) //line:vm:mm:begininit
        return -1;

    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);                     //line:vm:mm:endinit  
    printf("in init (start): %p\n", heap_listp);
/* $end mminit */

#ifdef NEXT_FIT
    rover = heap_listp;
#endif
/* $begin mminit */

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}
/* $end mminit */

/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

/* $end mmmalloc */
    if (heap_listp == 0){
        mm_init();
    }
/* $begin mmmalloc */
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

#ifdef WORD_ALIGNMENT
    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= WSIZE)                                          //line:vm:mm:sizeadjust1
        asize = WSIZE + DSIZE;
    else
        asize = WSIZE * ((size + (DSIZE) + (WSIZE-1)) / WSIZE); //line:vm:mm:sizeadjust3
#else 
    if (size <= DSIZE)                                          //line:vm:mm:sizeadjust1
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); //line:vm:mm:sizeadjust3
#endif

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {  //line:vm:mm:findfitcall
        place(bp, asize);                  //line:vm:mm:findfitplace
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 //line:vm:mm:growheap1
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;                                  //line:vm:mm:growheap2

    place(bp, asize);                                 //line:vm:mm:growheap3
    return bp;
}
/* $end mmmalloc */

/* 
 * mm_free - Free a block 
 */
/* $begin mmfree */
void mm_free(void *bp)
{
/* $end mmfree */
    if(bp == 0)
        return;

/* $begin mmfree */
    size_t size = GET_SIZE(HDRP(bp));
/* $end mmfree */
    if (heap_listp == 0){
        mm_init();
    }
/* $begin mmfree */

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/* $end mmfree */
/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin mmfree */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    } else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    } else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
/* $end mmfree */
#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) 
    rover = bp;
#endif
/* $begin mmfree */
    return bp;
}
/* $end mmfree */

/*
 * mm_realloc - Naive implementation of realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/* 
 * checkheap - We don't check anything right now. 
 */
void mm_checkheap(int verbose)
{
}

/* 
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;                                        //line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   //line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   //line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ //line:vm:mm:newepihdr

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          //line:vm:mm:returnblock
}
/* $end mmextendheap */

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));
    int minimum_block_size;

#ifdef WORD_ALIGNMENT
    minimum_block_size = WSIZE + DSIZE;
#else 
    minimum_block_size = 2 * DSIZE;
#endif

    if ((csize - asize) >= minimum_block_size) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
/* $begin mmfirstfit */
/* $begin mmfirstfit-proto */
static void *find_fit(size_t asize)
/* $end mmfirstfit-proto */
{
/* $end mmfirstfit */

#ifdef NEXT_FIT 
    /* Next fit search */
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    for ( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    return NULL;  /* no fit found */
#else
/* $begin mmfirstfit */
    /* we must determine which placement algorithm we will use */
    void *(*placementFunc)(size_t);

    placementFunc = isFirstFit ? &firstFit : &bestFit;
    return (*placementFunc)(asize);
#endif
}


void* firstFit(size_t asize) {
    /* First fit search */
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

void* bestFit(size_t asize) {
    /* Best fit search */
    int minFreeSpace = MAX_HEAP;
    int currentFreeSpace;
    void *bp, *bestFitBP;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        currentFreeSpace = GET_SIZE(HDRP(bp));
        if (!GET_ALLOC(HDRP(bp)) && (asize <= currentFreeSpace) && (currentFreeSpace < minFreeSpace)) {
            minFreeSpace = currentFreeSpace;
            bestFitBP = bp;

            /* we can make a slight optimization to this algorithm by determining
             * if the adjusted size is equal to the currentFreeSpace */
            if (asize == currentFreeSpace) break;
        }
    }

    /* if the minFreeSpace var stayed at MAX_HEAP then we clearly didn't find a free space */
    if (minFreeSpace < MAX_HEAP)
        return bestFitBP;

    return NULL; /* No fit */
}


static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }
	printf("%zu\t%s\t%p\t%p\n", hsize-2*WSIZE, (halloc ? "yes" : "no"), bp+WSIZE, bp+hsize-WSIZE);

    /* printf("%p: header: [%zu:%c] footer: [%zu:%c] AND contents: %zu and start contents: %p and end contents: %p\n", bp,
    hsize, (halloc ? 'y' : 'n'), //payload w/padding plus foot+head
    fsize, (falloc ? 'y' : 'n'), 
	hsize-2*WSIZE,
	bp+WSIZE,
	bp+hsize-WSIZE); */
}



static void checkblock(void *bp)
{
    size_t block_alignment;

#ifdef WORD_ALIGNMENT
    block_alignment = WSIZE;
#else
    block_alignment = DSIZE;
#endif

    if ((size_t)bp % block_alignment)
        printf("Error: %p is not doubleword(in our case 4) aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(int verbose)
{
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");

    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);

        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}
