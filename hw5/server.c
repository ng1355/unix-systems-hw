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
	char portstr[6];
	while(1){
		printf("Enter a username (16 chars max): ");
		if(psgets(uname, UNAME_SIZE + 1) < 0) continue;
		printf("Enter port number: ");
		if(psgets(portstr, PORT_SIZE + 1) < 0 || 
			(*port = parse_port(portstr))
					< 0) continue;
		break;
	}
	strtok(uname, "\n");
}
