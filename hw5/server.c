/* Server code by Nikita Georgiou
 * Hosts a server on user specified port with a chosen username.
 * the server will listen for incoming connections, and chat with the first
 * one to connect. If the client quits, the server will resume listening for
 * additional clients. SIGINT (^C) Can be used to quit the program */ 

#include "communication.h"
#include "common.h"

const char *PROGRAM_NAME;
extern int errno;

void config(uint16_t *port, char *uname);

int main(int argc, char** argv){
	(void) argc; 
	PROGRAM_NAME = argv[0];

	int sock, clientsock;
	uint16_t port;
	char uname[UNAME_SIZE];

	/* configure settings from command line */ 
	config(&port, uname);

	/* set up sockety stuff */ 
	if((sock = server_socket_setup(port)) < 0) exit(EXIT_FAILURE);

	if(listen(sock, BACKLOG_MAX) < 0){
		fprintf(stderr, "%s: failed to listen\n", PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}

	/* main loop, accepts an incoming connection, chats, then resumes
	 * waiting for subseqeuent clients */ 
	while(1){
		puts("Waiting for connections...");
		if((clientsock = establish_client(sock)) < 0) exit(EXIT_FAILURE);
		puts("Connection accepted! You are now chatting.");
		chat(clientsock, uname);
	}
}

void config(uint16_t *port, char *uname){
	char portstr[7];
	int port_test;

	while(1){
		/* PAD to accomodate the newline and null bit */ 
		printf("Enter a username (16 chars max): ");
		if(psgets(uname, UNAME_SIZE + PAD) < 0) continue;
		printf("Enter port number (Leave blank for any port): ");
		if(psgets(portstr, PORT_SIZE + PAD) < 0 || 
			(port_test =  parse_port(portstr)) < 0) continue;
		break;
	}

	*port = (uint16_t) port_test;
	strtok(uname, "\n");
}
