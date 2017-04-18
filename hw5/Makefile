#Makefile for homework by nikita
FLAGS = -std=c99 -Wall -Werror -pedantic -Wextra
all: server client

server: server.o communication.o common.o
	gcc server.o communication.o common.o -o server

client: client.o communication.o common.o
	gcc client.o communication.o common.o -o client

client.o: client.c
	gcc ${FLAGS} -Ofast -c client.c

server.o: server.c 
	gcc ${FLAGS} -Ofast -c server.c

communication.o: communication.c
	gcc ${FLAGS} -Ofast -c communication.c

common.o: common.c
	gcc ${FLAGS} -Ofast -c common.c

clean:
	rm -f server d_server *.o
