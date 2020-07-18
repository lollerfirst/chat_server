#ifndef __QUEUE_H_
#define __QUEUE_H_ 1
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

typedef struct Elem_{
	struct sockaddr_in6 cli_addr;
	int cli_fd;
	struct Elem_* ptr;
} Elem;

extern Elem* head;
extern Elem* tail;

extern int queue_init(size_t);
extern int enqueue(Elem);
extern int dequeue(Elem*);
extern void queue_deinit();
#endif