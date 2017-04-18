/* Header file for communication functions by Nikita Georgiou.
 * Use in conjunction with server.c and client.c */ 

#ifndef NIKITA_COMMUNICATION_H
#define NIKITA_COMMUNICATION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define UNAME_SIZE 16
#define ADDR_SIZE 16
#define PORT_SIZE 5
#define PORT_MAX 65535
#define PORT_MIN 1
#define BACKLOG_MAX 5
#define MSG_MAX 256

/* ***************************************************************************
 * The following functions establish socket stuff, create structs, etc. 
 * necessary for communication
 * **************************************************************************/

/* takes a port and binds a socket for a server to listen on. Socket is of
 * type IPv4 and TCP, running on any address (INADDR_ANY). Returns socket fd
 * or -1 on error. */ 
int server_socket_setup(uint16_t);

/* creates a socket fd bound to TCP connected IPv4 address ip through
 * port port. Returns socket fd or -1 on error, printing error messages */ 
int client_socket_setup(char *ip, uint16_t port);

/* Accepts a connection from a client, setting up requisite structs for the 
 * new socket. Returns client socket that this server should actually use for
 * communication. Returns -1 on error. */ 
int establish_client(int);

/* takes a port number as a char* and parses it into an int. The port is
 * then checked to be within the range of an unsigned 16 bit number, returning 
 * -1 if not. Non-numeric chars are ignored. If the provided string is  
 * unparseable (no numeric chars), then the function fails, returning -1. 
 * Note that the return type is expected to be checked by the caller for -1
 * before safely being cast into something like a uint16_t, as non-erronious
 * returns are guarinteed to be within this range. */ 
int parse_port(char*);

/* ***************************************************************************
 * The following functions deal with sending and recieving data
 * **************************************************************************/

/* Uses pselect to choose between reading from stdin to send a message to
 * a client, or read from a client on a pending socket. Note that message
 * send size includes username size and prompt formatting. */ 
void chat(int, char*);

/* reads from the given socket fd into the provided
 * char buffer up to size bytes. Prints error messages and also handles
 * shutting down the socket fd on failure. Returns 0 on success and -1
 * on failure. */ 
int read_from_client(int, char*, size_t);

/* flushes stdout. this creates a formatted message where the sender's
 * username is appended to the beginning of the message. Internally
 * handles shutting down sockets on communicate failure. Returns 0 on
 * success and -1 on failure */ 
int send_to_client(int, char*, char*, size_t);

#endif
