#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#define PORT 1800
#define LOCALHOST "127.0.0.1"
#define MAX_LINE 1024

pthread_t listen_thread;
pthread_mutex_t mutex;
int socket_fd;

void* listen_routine(void* arg){
	char buffer[MAX_LINE+1];
	while(read(socket_fd, buffer, sizeof(buffer)) > 0){
		pthread_mutex_lock(&mutex);
		printf("%s\n", buffer);
		pthread_mutex_unlock(&mutex);
	}
	
	perror(strerror(errno));
	return NULL;
}

int main(){
	struct sockaddr_in6 address;
	bzero(address, sizeof(struct sockaddr_in6));
	char buffer[MAXLINE+1];
	
	address.sin6_family = AF_INET6;
	inet_pton(AF_INET6, LOCALHOST, &address.sin6_addr.in6_addr.s6_addr);
	address.sin6_port = htons(PORT);
	
	if((socket_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){
		perror(strerror(errno));
		exit(errno);
	}
	
	if(connect(socket_fd, (struct sockaddr*) &address, sizeof(struct sockaddr_in6)) < 0){
		perror(strerror(errno));
		close(socket_fd);
		exit(errno);
	}
	
	pthread_mutex_init(&mutex);
	pthread_create(&listen_thread, NULL, &listen_routine, NULL);
	signal(SIGINT, &quit_handler);
	
	printf("Successfully Connected!\n");
	while(1){
		bzero(buffer, sizeof(buffer));
		pthread_mutex_lock(&mutex);
		printf(">");
		pthread_mutex_unlock(&mutex);
		scanf("%s", buffer);
		pthread_mutex_lock(&mutex);
		printf("\n");
		pthread_mutex_unlock(&mutex);
		
		
		if(write(socket_fd, buffer, sizeof(buffer)) <= 0){
			perror(strerror(errno));
			close(socket_fd);
			quit_handler(errno);
		}
		
	}
	
	
	
	return 0;
}