/* Client code by Nikita Georgiou
 * Asks user for an ip and port to connect to. Can then exchange messages
 * with a server. Can be quit with ^C */ 
#include <stdio.h>
#include <sys/socket.h>
#include "communication.h"
#include "common.h"

const char *PROGRAM_NAME;

int main(int argc, char** argv){
	(void) argc;
	PROGRAM_NAME = argv[0];

	int sock;
	/* ADDR + 1 to accomodate null */
	char ip[ADDR_SIZE + 1], uname[UNAME_SIZE];
	uint16_t port = 0; /* an optional port is odd... */
	strcpy(ip, "127.0.0.1"); /* copy of literal so should be safe */

	/* TODO: refactor
	 * Allows an optional port and IP, in either order. IPs are 
	 * understood as having periods. the check for "localhost"
	 * is a nop as ip begins as localhost */ 
	switch(argc){
		case 4: 
			if(strchr(argv[3], '.') != NULL)
				strncpy(ip, argv[3], ADDR_SIZE + 1);
			else if(strncmp(argv[3], "localhost", 9) == 0);
			else{
				if((port = parse_port(argv[3])) < 1)
					exit(EXIT_FAILURE);
			}
		case 3:
			if(strchr(argv[2], '.') != NULL)
				strncpy(ip, argv[2], ADDR_SIZE + 1);
			else if(strncmp(argv[2], "localhost", 9) == 0);
			else{
				if((port = parse_port(argv[2])) < 1)
					exit(EXIT_FAILURE);
			}
		case 2: 
			if(strnlen(strncpy(uname, argv[1], UNAME_SIZE - 1),
				UNAME_SIZE) < strnlen(argv[1], UNAME_SIZE + 1))
				fprintf(stderr, "Username truncated to %s\n",
						uname);
			break;
		default:
			fprintf(stderr, "Usage: username [addr | port]\n");
			exit(EXIT_FAILURE);
	}

	/* set up socket to connect to server */ 
	if((sock = client_socket_setup(ip, port)) < 0) exit(EXIT_FAILURE);

	/* send msg to server from stdin, echo own msg & other msgs to stdout */
	puts("Connected, happy chatting!\n Type \":dc\" to disconnect");
	chat(sock, uname);
}
