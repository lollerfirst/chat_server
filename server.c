
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
#include <bool.h>
#include <errno.h>
#include <assert.h>

#define PORT 1800
#define MAX_CLIENTS 20
#define MAX_LINE 512
#define SERVER_BACKLOG 100

//TYPE DEFINITIONS

typedef struct __Client{
	struct sockaddr_in6 cli_addr;
	int cli_fd;
	bool busy;
	pthread_t thread;
	pthread_mutex_t lock;
} Client;

//GLOBAL VARIABLES

static Client clients[MAX_CLIENTS];

void quit_handler(int c)
{
	
}

int broadcast(const char *message, size_t size)
{
	int i;
	for (i=0; i<MAX_CLIENTS; ++i){
		pthread_mutex_lock(&clients[i].lock);

		if (clients[i].busy)
		{
			if (write(clients[i].cli_fd, message, size) < 0)
			{
				perror(strerror(errno));
				pthread_mutex_unlock(&clients[i].lock);
				return -1;
			}

		}

		pthread_mutex_unlock(&clients[i].lock);
	}

	return 0;
}

void* conn_routine(void* arg)
{

	// Sanity check
	assert(arg != NULL);

	// Install the signal handlers
	struct sigaction sigact = {0};
	sigact.sa_handler = quit_handler;
	sigaction(SIGUSR1, &sigact, NULL);
	sigaction(SIGINT, &sigact, NULL);

	// Vars
	size_t iterator = *(size_t*) arg;
	ssize_t bytes;
	char buffer[MAX_LINE + 1] = {0};
	char from[64] = {0};
	inet_ntop(AF_INET6, &clients[iterator].cli_addr, from, sizeof(from));

	// only this thread can READ from cli_fd
	while ((bytes = read(clients[iterator].cli_fd, buffer, MAX_LINE)) > 0)
	{
		//Append sender address to message
		char message[1024] = {0};
		sprintf(message, "<%s>: %s", from, buffer);

		//Broadcast message to other clients
		(void) broadcast(message, sizeof(message));
	}

	if (bytes == -1)
	{
		perror(strerror(errno));
	}

	// Other threads may be trying to write to cli_fd
	pthread_mutex_lock(&clients[iterator].lock);

	clients[iterator].busy = false;
	close(clients[iterator].cli_fd);

	pthread_mutex_unlock(&clients[iterator].lock);
}

int main()
{
	struct sockaddr_in6 address;
	size_t addrlen = sizeof(address);
	size_t iterator = 0;
	int listen_fd;

	// INITIALIZING STUFF
	bzero(&address, sizeof(struct sockaddr_in6));
	bzero(clients, sizeof(clients));

	size_t k;
	for (k=0; k<MAX_CLIENTS; ++k){
		clients[k].lock = PTHREAD_MUTEX_INITIALIZER;
	}

	// SET UP CONNECTION TYPE IPv6, HOST IP, PORT
	address.sin6_family = AF_INET6;
	address.sin6_addr.in6_addr.s6_addr = htonl(INADDRESS_ANY);
	address.sin6_port = htons(PORT);

	// CREATE THE SOCKET
	if ((listen_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
	{
		perror(strerror(errno));
		exit(errno);
	}

	// Bind the socket
	if (bind(listen_fd, (struct sockaddr *)&address, addrlen) == -1)
	{
		perror(strerror(errno));
		close(listen_fd);
		exit(errno);
	}

	// Mark the socket as passive
	if (listen(listen_fd, SERVER_BACKLOG) == -1)
	{
		perror(strerror(errno));
		close(listen_fd);
		exit(errno);
	}

	// Listen for and enqueue connections
	while (1)
	{
		struct sockaddr_in6 cli_addr = {0};
		socklen_t addrlen = sizeof(struct sockaddr_in6);

		int cli_fd;
		if ((cli_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &addrlen)) == -1)
		{
			perror(strerror(errno));
			kill(0, SIGUSR1);
			close(listen_fd);

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
				close(cli_fd);
			}

			pthread_mutex_unlock(&clients[iterator].lock);
		}
		else
		{
			close(cli_fd);
		}
	}

	return 0;
}

