#include <stdio.h>
#include <stdlib.h>
#include "map.h"


int insert_node(node_t * head, int index, char *bp) {
    node_t * current = head;

    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->index = index;
    current->next->bp = bp;
    current->next->next = NULL;
    return index;
}

char * find_node(node_t * head, int index){
    node_t * current = head;

    while (current != NULL) {
        if (current->index == index){
            return current->bp;
        }
        current = current->next;
    }
    return NULL;
}

int remove_by_index(node_t * head, int n) {
    int i = 0;
    int retval = -1;
    node_t * current = head;
    node_t * temp_node = NULL;

    if (n == 0) {
        return pop(head);
    }

    for (; i < n-1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->index;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        printf("key %d\n", current->index);
        printf("value %p\n", current->bp);
        current = current->next;
    }
}

int pop(node_t * head) {
    int retval = -1;
    node_t * next_node = NULL;

    if (head == NULL) {
        return -1;
    }

    next_node = (head)->next;
    retval = (head)->index;
    free(head);
    head = next_node;

    return retval;
}

void free_list(node_t * head) {
	node_t * to_del;
	node_t * current;
	to_del	= head;
	while(to_del->next != NULL){
		current = to_del->next;
		free(to_del);
		to_del = current;	
	}

	
}
