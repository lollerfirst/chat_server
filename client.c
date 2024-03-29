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
int socket_fd;

void* listen_routine(void* arg){
	char buffer[MAX_LINE+1] = {0};

	while(read(socket_fd, buffer, MAX_LINE) > 0){

		fprintf(stdout, "\n%s\n", buffer);

		bzero(buffer, sizeof(buffer));
		
	}

	fprintf(stdout, "\n[SERVER DISCONNECTED]\n");
	return NULL;
}

int main(int argc, char** argv){

	if(argc < 3) {
		fprintf(stderr, "USAGE: ./chat <SERVER_IP> <PORT>\n");
		return -1;
	}

	struct sockaddr_in address;
	bzero(&address, sizeof(struct sockaddr_in));
	
	address.sin_family = AF_INET;
	if (inet_pton(AF_INET, argv[1], &address.sin_addr) != 1){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	};
	
	short port_tmp;
	if ((port_tmp = (short) atoi(argv[2])) == 0){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	}
	
	address.sin_port = htons(port_tmp);
	
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d", __LINE__);
		return -1;
	}
	
	ERROR_RETHROW(
		connect(socket_fd, (struct sockaddr*) &address, sizeof(struct sockaddr_in)),
		perror(strerror(errno)),
		fprintf(stderr, "LINE: %d", __LINE__),
		close(socket_fd)
	);

	ERROR_RETHROW(
		pthread_create(&listen_thread, NULL, &listen_routine, NULL),
		perror(strerror(errno)),
		fprintf(stderr, "LINE: %d", __LINE__),
		close(socket_fd)
	);

	printf("[+] Successfully Connected!\n");

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