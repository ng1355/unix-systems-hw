/* Server code by Nikita Georgiou
 * Hosts a server on user specified port with a chosen username.
 * the server will listen for incoming connections, and chat with the first
 * one to connect. If the client quits, the server will resume listening for
 * additional clients. SIGINT (^C) Can be used to quit the program */ 

#include "communication.h"
#include "common.h"

const char *PROGRAM_NAME;
extern int errno;

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];

	int sock, clientsock;
	uint16_t port = 0;
	char uname[UNAME_SIZE];

	switch(argc){
		case 3:
			if((port = parse_port(argv[2])) < 1)
				exit(EXIT_FAILURE);
		case 2: 
			if(strnlen(strncpy(uname, argv[1], UNAME_SIZE - 1),
				UNAME_SIZE) < strnlen(argv[1], UNAME_SIZE + 1))
				fprintf(stderr, "Username truncated to %s\n",
						uname);
			break;
		default:
			fprintf(stderr, "Usage: username [port]\n");
			exit(EXIT_FAILURE);
	}

	/* set up sockety stuff */ 
	if((sock = server_socket_setup(&port)) < 0) exit(EXIT_FAILURE);

	printf("Listening on port %d...\n", port);

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
		puts("Type \":dc\" to disconnect");
		chat(clientsock, uname);
	}
}
