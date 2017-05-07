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

#define CHAT_DC -1 /* chat control, tells the chat to close the sock and return */
#define UNAME_SIZE 16 /* arbitrary, username length */
#define ADDR_SIZE 15 /* abc.def.ghi.jkl = 15 */
#define PORT_SIZE 5 /* 2 byte number can have up to 5 digits (see PORT_MAX) */
#define PORT_MAX 65535 /* upper bound of uint16_t */ 
#define PORT_MIN 1 /* lower bound + 1 */
#define BACKLOG_MAX 5 /* arbitrary, for listen's backlog option */ 
#define MSG_MAX 256 /* arbitrary, for message length, incl. uname + formatting */
/* arbitrary. the max number of users allowed in a chat before server rejects
 * additional connections */ 
#define MAX_CONN 25
/* when a client joins a server, they're given a list of currently
 * connected users. The size of the buffer to hold this info is
 * therefor at an upper bound of the max number of connected users
 * times the max username size. The 50 is arbitrary padding in case
 * flavor text (eg. "Connected users: ") is also provided */ 
#define UNAMELIST_SIZE (MAX_CONN * UNAME_SIZE) + 50 

/* ***************************************************************************
 * the following are control functions that change how the chat operates
 * internally, local to the user
 * **************************************************************************/

/* if the user begins a message with ':', all following input will be parsed
 * as a chat command. The returned int defines what the chat should do next */
int chat_control(char *msg);

void send_uname(int sock, char *uname);

void recv_unamelist(int sock, char *userlist);

/* ***************************************************************************
 * The following functions establish socket stuff, create structs, etc. 
 * necessary for communication
 * **************************************************************************/

/* takes a port ptr and binds a socket for a server to listen on. Socket is of
 * type IPv4 and TCP, running on any address (INADDR_ANY). Returns socket fd
 * or -1 on error. Note that the passed port is modified by the function
 * so that, in the case 0 is passed, the bound port number can be echoed by the
 * caller */ 
int server_socket_setup(uint16_t*);

/* creates a socket fd bound to TCP connected IPv4 address ip through
 * port port. Returns socket fd or -1 on error, printing error messages */ 
int client_socket_setup(char *ip, uint16_t port);

/* Accepts a connection from a client, setting up requisite structs for the 
 * new socket. Returns client socket that this server should actually use for
 * communication. Returns -1 on error. */ 
int establish_client(int);

/* takes a port number as a char* and parses it into an int. The port is
 * then checked to be within the range of an unsigned 16 bit number, returning 
 * 0 if not. Non-numeric chars are ignored. If the provided string is  
 * unparseable (no numeric chars), then the function fails, returning 0. */
uint16_t  parse_port(char*);

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

/* this creates a formatted message where the sender's username 
 * is appended to the beginning of the message. Internally
 * handles shutting down sockets on communicate failure. Returns 0 on
 * success and -1 on failure. Reads up to at most MSG_MAX characters, including
 * username and formatting length. Longer messages are truncated */ 
int send_to_client(int, char*, char*);

#endif
