#Makefile for homework by nikita
FLAGS = -std=c99 -Wall -Wextra -Werror -pedantic 
D_FLAGS = -std=c99 -Wall -Wextra -fno-omit-frame-pointer -g

all: server client
debug: server_debug client

server: server.o communication.o common.o
	gcc server.o communication.o common.o -o server

client: client.o communication.o common.o
	gcc client.o communication.o common.o -o client

server_debug: server_d.o communication.o common.o
	gcc server.o communication.o common.o -o server

client.o: client.c
	gcc ${FLAGS} -D_POSIX_C_SOURCE=200809L -Ofast -c client.c

server.o: server.c 
	gcc ${FLAGS} -pthread -D_POSIX_C_SOURCE=200809L -Ofast -c server.c

server_d.o: server.c
	gcc ${D_FLAGS} -pthread -D_POSIX_C_SOURCE=200809L -Ofast -c server.c

communication.o: communication.c
	gcc ${FLAGS} -D_POSIX_C_SOURCE=200112L -Ofast -c communication.c

common.o: common.c
	gcc ${FLAGS} -D_POSIX_C_SOURCE=200809L -Ofast -c common.c

clean:
	rm -f server client *.o
cleanobj: 
	rm -f *.o
