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
#include <signal.h>

#define MAX_LINE 1024
#define MAX_MESSAGE 512

#define ERROR_RETHROW(__CALL, __STATEMENTS...) ({	\
	int __err_code;									\
	if ((__err_code = __CALL) != 0) 				\
	{ 												\
		{__STATEMENTS;} 							\
		return __err_code; 							\
	} 												\
})

pthread_t listen_thread;
pthread_mutex_t mutex;
int socket_fd;

void* listen_routine(void* arg){
	char buffer[MAX_LINE+1] = {0};

	while(read(socket_fd, buffer, MAX_LINE) > 0){

		pthread_mutex_lock(&mutex);
		fprintf(stdout, "%s\n", buffer);
		pthread_mutex_unlock(&mutex);

		bzero(buffer, sizeof(buffer));
		
	}

	fprintf(stdout, "[SERVER DISCONNECTED]\n");
	return NULL;
}

int main(int argc, char** argv){

	if(argc < 3) {
		fprintf(stderr, "USAGE: ./chat <SERVER_IP> <PORT>\n");
		return -1;
	}

	char remote_server[256] = {0};
	char remote_port[32] = {0};

	// input safety
	memcpy(remote_server, argv[1], sizeof(remote_server)-1);
	memcpy(remote_port, argv[2], sizeof(remote_port)-1);

	struct sockaddr_in6 address;
	bzero(&address, sizeof(struct sockaddr_in6));
	
	address.sin6_family = AF_INET6;
	if (inet_pton(AF_INET6, remote_server, &address.sin6_addr.s6_addr) != 1){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	};
	
	short port_tmp;
	if ((port_tmp = (short) atoi(remote_port)) == 0){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	}
	
	address.sin6_port = htons(port_tmp);
	
	if ((socket_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	}
	
	ERROR_RETHROW(
		connect(socket_fd, (struct sockaddr*) &address, sizeof(struct sockaddr_in6)),
		perror(strerror(errno)),
		fprintf(stderr, "LINE: %d", __LINE__),
		close(socket_fd)
	);

	pthread_mutex_init(&mutex, NULL);

	ERROR_RETHROW(
		pthread_create(&listen_thread, NULL, &listen_routine, NULL),
		perror(strerror(errno)),
		fprintf(stderr, "LINE: %d", __LINE__),
		close(socket_fd)
	);

	pthread_mutex_lock(&mutex);
	printf("Successfully Connected!\n");
	pthread_mutex_unlock(&mutex);

	char* buffer;
	size_t buffer_len = MAX_LINE;

	if ((buffer = calloc(sizeof(char), (MAX_LINE+1))) == NULL){
		pthread_kill(listen_thread, SIGINT);
		close(socket_fd);
		return -1;
	}


	while(1){
		
		if (getline(&buffer, &buffer_len, stdin) == -1){
			perror(strerror(errno));
			free(buffer);
			pthread_kill(listen_thread, SIGINT);
			close(socket_fd);
			return -1;
		}
		
		if (write(socket_fd, buffer, strlen(buffer)-1) <= 0){
			perror(strerror(errno));
			pthread_kill(listen_thread, SIGINT);

			close(socket_fd);
			return -1;
		}
		
		bzero(buffer, sizeof(buffer));
	}	
	
	
	return 0;
}