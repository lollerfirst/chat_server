queue.o: queue.c queue.h
	gcc -c queue.c
server.o: server.c queue.h
	gcc -c server.c

server: server.o queue.o
	gcc server.o queue.o -lpthread -o server

clean:
	rm *.o server client

all:
	make server client