#Makefile for homework by nikita
FLAGS = -std=c99 -Wall -Werror -pedantic
all: sh

sh: sh.c
	gcc ${FLAGS} -Ofast -o sh sh.c 

debug: sh.c
	gcc ${FLAGS} -O1 -fno-omit-frame-pointer -g -o d_sh sh.c

clean:
	rm -f sh d_sh
