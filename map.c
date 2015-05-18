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
    current->next->written_heap_size = 0;
    current->next->next = NULL;
    return index;
}

node_t * find_node(node_t * head, int index){
    node_t * current = head;

    while (current != NULL) {
        if (current->index == index){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int remove_by_index(node_t * head, int n) {
   
    int retval = -1;
    node_t * current = head;
    node_t * temp = NULL;

	while(current != NULL && current->index != n){
	    temp = current;
	    current = current->next;
	}
	//now we are either in shits pond or at the front or have the block number in question
	if(current !=NULL){
		retval = current->index;
		if (temp == NULL) {// at the front
			head = current->next;
	    } else {
			temp->next = current->next;
        }

    free(current);
}
    return retval;
}

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
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
