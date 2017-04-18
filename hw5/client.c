/* Client code by Nikita Georgiou
 * Asks user for an ip and port to connect to. Can then exchange messages
 * with a server. Can be quit with ^C */ 
#include <stdio.h>
#include <sys/socket.h>
#include "communication.h"
#include "common.h"

const char *PROGRAM_NAME;

void config(char *addr, uint16_t *port, char *uname);

int main(int argc, char** argv){
	(void) argc;
	PROGRAM_NAME = argv[0];

	int sock;
	char ip[ADDR_SIZE + 1], uname[UNAME_SIZE + 1];
	uint16_t  port;
	/* read username, ip, and port from user */
	config(ip, &port, uname);
	/* set up socket to connect to server */ 
	if((sock = client_socket_setup(ip, port)) < 0) exit(EXIT_FAILURE);

	/* send msg to server from stdin, echo own msg & other msgs to stdout */
	puts("Connected, happy chatting!");
	//getchar();
	chat(sock, uname);
}

void config(char *addr, uint16_t *port, char *uname){
	char portstr[PORT_SIZE + PAD];
	int port_test;
	while(1){
		printf("Enter a username (16 chars max): ");
		if(psgets(uname, UNAME_SIZE + PAD) < 1) continue;
		printf("Enter an IPv4 address (Leave blank for localhost): ");
		if(psgets(addr, ADDR_SIZE + PAD) < 0) continue;
		printf("Enter port number: ");
		/* while leaving the port field blank is not invalid like
		 * in the server, unlike the server its almost guarinteed to 
		 * fail to connect to something */ 
		if(psgets(portstr, PORT_SIZE + 2) < 0 ||
				(port_test =  parse_port(portstr)) < 0)
			continue;
		break;
	}

	*port = (uint16_t) port_test;
	/* uname and addr are guarinteed to be chopped down to macro + 1 here */
	strtok(uname, "\n");
	strtok(addr, "\n");

	if(strncmp(addr, "localhost", 9) == 0 || addr[0] == '\n')
		strncpy(addr, "127.0.0.1", 10);
}
