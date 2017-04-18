#Makefile for homework by nikita
FLAGS = -std=c99 -Wall -Werror -pedantic -Wextra
all: server client cleanobj

server: server.o communication.o common.o
	gcc server.o communication.o common.o -o server

client: client.o communication.o common.o
	gcc client.o communication.o common.o -o client

client.o: client.c
	gcc ${FLAGS} -Ofast -c client.c

server.o: server.c 
	gcc ${FLAGS} -Ofast -c server.c

communication.o: communication.c
	gcc ${FLAGS} -D_POSIX_C_SOURCE=200112L -Ofast -c communication.c

common.o: common.c
	gcc ${FLAGS} -D_POSIX_C_SOURCE=200809L -Ofast -c common.c

clean:
	rm -f server client *.o
cleanobj: 
	rm -f *.o
