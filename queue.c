#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#define UNINIT -999

typedef struct Elem_{
	struct sockaddr_in6 cli_addr;
	int cli_fd;
	struct Elem_* ptr;
} Elem;

Elem* head;
Elem* tail;
size_t queue_len;
int MAX_SIZE = UNINIT;

int queue_init(size_t max){
	if(MAX_SIZE == UNINIT){
		MAX_SIZE = max;
		queue_len = 0;
		return 0;
	}
	
	return -1;
}

int enqueue(Elem el){
	if(queue_len >= MAX_SIZE || MAX_SIZE == UNINIT) return -1;
	
	Elem* tmp = malloc(sizeof(Elem));
	*tmp = el;
	tmp->ptr = NULL;
	
	if(queue_len == 0){
		head = tmp;
		tail = tmp;
		queue_len++;
	}else{
		tail->ptr = tmp;
		tail = tmp;
		queue_len++;
	}
	
	return 0;
}

int dequeue(Elem* el){
	if(queue_len <= 0)
		return -1;
	
	
	if(el != NULL) *el = *head;
	Elem* tmp = head;
	head = head->ptr;
	free(tmp);
	queue_len--;
	
	return 0;
}

void queue_deinit(){
	while(dequeue(NULL) == 0);
	MAX_SIZE = UNINIT;
}
