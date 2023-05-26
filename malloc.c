#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>

struct node{
    int size;
    int free;
    struct node *next;
	void *start;
};
struct node* head = NULL;

struct node* check(size_t request) {
    struct node *current = head;
    while (current != NULL) {
        if (current->size >= request && current->free == 1) {
            return current;
        }
        current = current->next; 
    }
    return NULL;
}	

void split(struct node* currNode, int request){
    struct node* nextNode;
    
    nextNode = currNode->start + request;
    nextNode->size = currNode->size - request - sizeof(struct node);
    nextNode->start = nextNode + 1;
    nextNode->next = currNode->next;
    nextNode->free = 1;

    currNode->size = request;
    currNode->free = 0;
    currNode->next = nextNode;
}


void *malloc( size_t size ){
	int remainder = size % 8;
	if (remainder){
		size += remainder;
	}
	
	int newSize = size + sizeof(struct node);
	long pageSize = sysconf(newSize);
	int request = ((newSize/pageSize)+1)*pageSize;
	
	//First element in linked list (NULL)
	struct node* newNode = sbrk(request);
	struct node* start = newNode;
	if(newNode == NULL){
		errno = ENOMEM;
		return NULL;
	}
	//Step 1
	if(head == NULL){
		struct node *newAlloc = newNode;
		newAlloc->size = size;
		newAlloc->free = 0;
		newAlloc->next = NULL;
		newAlloc->start = newAlloc + 1;
		head = newAlloc;
	}
	//Step 2
	struct node *ret = check(size);
	if (ret != NULL) {
        if (ret->size > (request + sizeof(struct node) + 1)) {
            split(ret, request);
        }
        else {
            ret->free = 0;
        }   
        return ret->start;
    }
		

		
	//Step 3
	newNode = sbrk(request);
	if(newNode == NULL){
		errno = ENOMEM;
		return NULL;
	}
	
	//adding node to end of list
	struct node *current = head;
    while (current != NULL) {
        if (current->next == NULL){
            current->next = newNode;
            break;
        }
        current = current->next;
    }
	
	//C pointer stuff
	newNode->start = newNode +1;
	newNode->size = request - sizeof(struct node);
	newNode->free = 1;
	newNode-> next = NULL;
	
	if(newNode->size > (size + sizeof(struct node) +1)){
		split(newNode, size);
	}else{
		newNode->free = 0;
	}
	return newNode->start;
};
	
	
void free( void *ptr ){
	struct node *current = head;
	struct node *wantedNode;
	if(ptr != NULL){
		while (current != NULL) {
			if (current->start == ptr){
				wantedNode = current;
				break;
			}
			current = current->next;
        }
    }
    if(wantedNode != NULL){
		wantedNode->free = 1;
	}
};


void *calloc( size_t num_of_elts, size_t elt_size ){
	struct node *current;
	size_t request = num_of_elts * elt_size;
	current = malloc(request);
	memset(current, 0, request);
};

void *realloc( void *pointer, size_t size){
 
    //get the node.
    struct node *curr = head;
    struct node *wantedNode ;
 
    while (curr!=NULL){
        if (curr-> start == ptr){
            wantedNode = curr;
            break;
        }
        curr = curr->next;
    }
    if (wantedNode != NULL){
        if (size == wantedNode->size){
            return pointer;
        }
        if (size < wantedNode->size){
            void* allocation = malloc(size);
            memcpy(allocation, wantedNode->start, size);
            free(pointer);
            return allocation;
        }
        if (size > wantedNode->size){
            void* allocation = malloc(size);
            memcpy(allocation, wantedNode->start, wantedNode->size);
            free(pointer);
            return allocation;
        }
    }
};
