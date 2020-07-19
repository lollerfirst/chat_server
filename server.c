/* TODO LIST:
	-add a enum for error enumeration
	-add conn_handler
	-add quit_handler

*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "queue.h"
#define PORT 1800
#define MAX_CLIENTS 20
#define MAX_LINE 1024
#define SERVER_BACKLOG 100

pthread_mutex_t mutex;
pthread_cond_t cond_var;
pthread_t pool[MAX_CLIENTS];
pthread_t mex_thread;
pthread_cond_t mex_cond_var;
Elem connected_clients[MAX_CLIENTS];
size_t conn_cli_len;

char message[MAX_LINE+1];
Elem last_cli;

void vector_push(void* vector, const void* element, size_t vector_len);    //to implement
void vector_remove(void* vector, const void* element, size_t vector_len);  //to implement

//Close everything on SIGINT signal
void quit_handler(int c){
	
	//Cancel all the threads
	int i;
	for(i=0; i<MAX_CLIENTS; i++)
		pthread_cancel(pool[i]);
	
	pthread_cancel(mex_thread);
	
	//Close all connections
	for(i=0; i<conn_cli_len; i++)
		close(connected_clients[i].cli_fd);
	
	//Deinit the pending connection queue
	queue_deinit();
	
	//Destroy mutexes and condition variables
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_var);
	pthread_cond_destroy(&mex_cond_var);
	
	//Exit the process with given value
	exit(c);
}

void* conn_handler(void* arg){
	Elem el;
	char buffer[MAX_LINE+1];  
	
	while(1){
		bzero(&el, sizeof(Elem));
		bzero(buffer, sizeof(buffer));
		pthread_mutex_lock(&mutex);
		
		//Check if there is an element on the queue. If there is not wait for the signal
		if(dequeue(&el) < 0){
			pthread_cond_wait(&cond_var, &mutex);
			dequeue(&el);	
		}
		vector_push(connected_clients, &el, &conn_cli_len);
		pthread_mutex_unlock(&mutex);
		
		while(read(el.cli_fd, buffer, sizeof(buffer)) > 0){
			pthread_mutex_lock(&mutex);
			last_cli = el;
			memcpy(message, buffer, sizeof(buffer));
			pthread_mutex_unlock(&mutex);
			
			pthread_cond_signal(&mex_cond_var);
		} 
		
		//Client has disconnected or there has been an error.
		close(el.cli_fd);
		pthread_mutex_lock(&mutex);
		vector_remove(connected_clients, el, &conn_cli_len);
		pthread_mutex_unlock(&mutex);
	}
}

//Once notified that there is a new message, it takes it and re-broadcasts it to all clients.
void* mex_handler(void* arg){
	char buffer[MAX_LINE+1];
	size_t buf_len = sizeof(buffer);
	int i;
	
	
	while(1){
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&mex_cond_var, &mutex);
		inet_ntop(AF_INET6, (struct in6_addr*) &last_cli.cli_addr.sin6_addr.in6_addr, buffer, buf_len); 
		strcat(buffer, ": ");
		strcat(buffer, message);
		strcat(buffer, '\0');
		
		
		for(i=0; i<conn_cli_len; i++)
			write(connected_clients[i].cli_fd, buffer, sizeof(buffer));
		
		pthread_mutex_unlock(&mutex);
		bzero(buffer, sizeof(buffer));
	}
}

int main(int argc, char** argv){
	struct sockaddr_in6 address, cli_addr;
	size_t addrlen = sizeof(address);
	int listen_fd;
	int iter;
	bzero(&addr, sizeof(struct sockaddr_in));
	bzero(&cli_addr, sizeof(struct sockaddr_in));
	bzero(&connected_clients, sizeof(connected_clients));
	conn_cli_len = 0;
	
	//Setting up the SIGINT handler
	signal(SIGINT, &quit_handler);
	
	//Set the address
	address.sin6_family = AF_INET6;
	address.sin6_addr.in6_addr.s6_addr = htonl(INADDRESS_ANY);
	address.sin6_port = htons(PORT);
	
	//Create the socket
	if((listen_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1){
		perror(strerror(errno));
		exit(errno);
	}
	
	//Bind the socket
	if(bind(listen_fd, (struct sockaddr*) &address, addrlen) == -1){
		perror(strerror(errno));
		close(listen_fd);
		exit(errno);
	}
	
	//Mark the socket as passive
	if(listen(listen_fd, SERVER_BACKLOG) == -1){
		perror(strerror(errno));
		close(listen_fd);
		exit(errno);
	}
	
	//Initialize the threads and threads vars
	pthread_mutex_init(&mutex);
	pthread_cond_init(&cond_var);
	for(iter = 0; iter < MAX_CLIENTS; iter++)
		pthread_create(pool+iter, NULL, &conn_handler, NULL);
	
	pthread_cond_init(&mex_cond_var);
	pthread_create(&mex_thread, NULL, &mex_handler, NULL);
	
	//Initialize the pending connections queue
	if(queue_init(SERVER_BACKLOG) < 0){
		perror("Queue couldn't initialize!\n");
		close(listen_fd);
		sigint_handler(-1);
	}
	
	//Wait for connections and enqueue them
	while(1){
		static struct sockaddr tmp;
		static Elem el;
		size_t cli_addr_len = sizeof(struct sockaddr);
		static int fd;
		if((fd = accept(listen_fd, &tmp, &cli_addr_len, 0)) < 0){
			perror(strerror(errno));
			sigint_handler(errno);
		}
		el.cli_addr = tmp.sockaddr_in6;
		el.cli_fd = fd;
		
		pthread_mutex_lock(&mutex);
		enqueue(el);
		pthread_mutex_unlock(&mutex);
		phtread_cond_signal(&cond_var); //signal that a connection has just been appeneded to the queue;
		
		bzero(&tmp, sizeof(struct sockaddr));
		bzero(&el, sizeof(Elem));
	}
	
	return 0;
}
