
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
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#define PORT 1800
#define LOOPBACK "0.0.0.0"
#define MAX_CLIENTS 20
#define MAX_LINE 512
#define SERVER_BACKLOG 100
#define CLIENT_DGRAM_SIZE 1024

#define ERROR_RETHROW(__CALL, __STATEMENTS...) ({	\
	int __err_code;									\
	if ((__err_code = __CALL) != 0) 				\
	{ 												\
		{__STATEMENTS;} 							\
		return __err_code; 							\
	} 												\
})

//TYPE DEFINITIONS

typedef struct __Client{
	struct sockaddr_in cli_addr;
	int cli_fd;
	bool busy;
	pthread_t thread;
	pthread_mutex_t lock;
} Client;

//GLOBAL VARIABLES

static Client clients[MAX_CLIENTS];
int listen_fd;

void quit_handler(int c)
{
	int i;
	for (i=0; i<MAX_CLIENTS; ++i){
		close(clients[i].cli_fd);
	}

	close(listen_fd);
}

void thread_quit_handler(int c){
	return;
}

int broadcast(const char *message, size_t size, size_t current_thread_id)
{
	size_t i;
	for (i=0; i<MAX_CLIENTS; ++i){
		pthread_mutex_lock(&clients[i].lock);

		if (i != current_thread_id && clients[i].busy)
		{
			if (write(clients[i].cli_fd, message, size) < 0)
			{
				perror(strerror(errno));
			}

		}

		pthread_mutex_unlock(&clients[i].lock);
	}

	fprintf(stdout, "%s\n", message);

	return 0;
}

void* conn_routine(void* arg)
{

	// Sanity check
	assert(arg != NULL);

	// Install the signal handlers
	struct sigaction sigact = {0};
	sigact.sa_handler = thread_quit_handler;
	sigaction(SIGUSR1, &sigact, NULL);
	sigaction(SIGINT, &sigact, NULL);

	// Vars
	size_t iterator = *(size_t*) arg;
	ssize_t bytes;
	char buffer[MAX_LINE + 1] = {0};
	char from[64] = {0};
	inet_ntop(AF_INET, &clients[iterator].cli_addr.sin_addr, from, sizeof(from));

	// only this thread can READ from cli_fd
	while ((bytes = read(clients[iterator].cli_fd, buffer, MAX_LINE)) > 0)
	{
		//Append sender address to message
		char message[CLIENT_DGRAM_SIZE];
		bzero(message, sizeof(message));

		sprintf(message, "<%s>: %s", from, buffer);

		//Broadcast message to other clients
		(void) broadcast(message, strlen(message), iterator);
		bzero(buffer, sizeof(buffer));
	}

	if (bytes == -1)
	{
		perror(strerror(errno));
	}

	char message[CLIENT_DGRAM_SIZE];
	bzero(message, sizeof(message));

	sprintf(message, "[-] Client Disconnected: <%s>", from);
	(void) broadcast(message, strlen(message), iterator);

	fprintf(stdout, "%s\n", message);

	// Other threads may be trying to write to cli_fd
	pthread_mutex_lock(&clients[iterator].lock);

	clients[iterator].busy = false;
	close(clients[iterator].cli_fd);

	pthread_mutex_unlock(&clients[iterator].lock);

}

int main()
{
	struct sockaddr_in address;
	size_t addrlen = sizeof(address);
	size_t iterator = 0;

	// INITIALIZING STUFF
	bzero(&address, sizeof(struct sockaddr_in));
	bzero(clients, sizeof(clients));

	size_t k;
	for (k=0; k<MAX_CLIENTS; ++k){
		pthread_mutex_init(&clients[k].lock, NULL);
	}

	// SET UP CONNECTION TYPE IPv6, HOST IP, PORT
	address.sin_family = AF_INET;
	int err;
	if ((err = inet_pton(AF_INET, LOOPBACK, &address.sin_addr)) != 1){
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d\n", __LINE__);
		return err;
	}
	address.sin_port = htons(PORT);

	// CREATE THE SOCKET
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror(strerror(errno));
		fprintf(stderr, "LINE: %d\n", __LINE__);
		exit(errno);
	}

	// Bind the socket
	ERROR_RETHROW(bind(listen_fd, (struct sockaddr *)&address, addrlen),

		perror(strerror(errno)),
		fprintf(stderr, "LINE: %d\n", __LINE__),
		close(listen_fd)
	);

	// Mark the socket as passive
	ERROR_RETHROW(listen(listen_fd, SERVER_BACKLOG),
	
		perror(strerror(errno)),
		close(listen_fd),
		fprintf(stderr, "LINE: %d\n", __LINE__),
		exit(errno)
	);

	//Install signal handler
	struct sigaction sigact = {.sa_handler = quit_handler};
	sigaction(SIGINT, &sigact, NULL);

	// Listen for and enqueue connections
	while (1)
	{
		struct sockaddr_in cli_addr;
		bzero(&cli_addr, sizeof(struct sockaddr_in));
		socklen_t addrlen = sizeof(struct sockaddr_in);

		int cli_fd;
		if ((cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &addrlen)) == -1)
		{
			perror(strerror(errno));
			kill(0, SIGUSR1);
			close(listen_fd);
			fprintf(stderr, "LINE: %d\n", __LINE__);

			exit(errno);
		}

		// find an available slot; only main thread can set busy to true
		size_t i;
		for (i=0; (clients[iterator].busy) && (i < MAX_CLIENTS); ++i)
			iterator = (iterator + 1) % MAX_CLIENTS;

		if (i < MAX_CLIENTS)
		{

			pthread_mutex_lock(&clients[iterator].lock);

			// append the client file descriptor to the queue and pass it to the thread
			clients[iterator].cli_addr = cli_addr;
			clients[iterator].cli_fd = cli_fd;
			clients[iterator].busy = true;

			//thread stack is independent from main process so iterator is copied into it presumably.
			if ((pthread_create(&clients[iterator].thread, NULL, conn_routine, (void*) &iterator) != 0))
			{
				clients[iterator].busy = false;
				perror(strerror(errno));
				fprintf(stderr, "LINE: %d\n", __LINE__);
				close(cli_fd);
			}

			pthread_mutex_unlock(&clients[iterator].lock);

			char from[64];
			inet_ntop(AF_INET, &clients[iterator].cli_addr.sin_addr, from, sizeof(from));
			char message[MAX_LINE] = {0};
			
			sprintf(message, "[+] Client Connected: <%s>", from);
			(void) broadcast(message, strlen(message), MAX_CLIENTS);

		}
		else
		{
			close(cli_fd);
		}
	}

	return 0;
}

