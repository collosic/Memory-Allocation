#ifndef MAP
#define MAP


typedef struct node {
    int index;
    char *bp;
    int written_heap_size;
    struct node * next;
} node_t;

int insert_node(node_t * head, int index, char *bp);
int remove_by_index(node_t * head, int n);
node_t * find_node(node_t * head, int index);
void print_list(node_t * head);
int pop(node_t * head);
void free_list(node_t * head);

#endif //MAP
