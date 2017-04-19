/* Implimentation file for communication.h by Nikita Georgiou */ 

#include "communication.h"
#include "common.h"

extern char *PROGRAM_NAME;

void chat(int sock, char *uname){
	fd_set input;
	char msg[MSG_MAX];

	while(1){
		/* reset pselect state so we're ready to accept from either
		 * stdin or the socket again */ 
		FD_ZERO(&input);
		FD_SET(sock, &input);
		FD_SET(fileno(stdin), &input);

		/* Clear msg incase input is terminated abruptly, 
		 * flush stdout because sockets tends to not play nice
		 * with buffered outputs (printf) */ 
		memset(msg, 0, MSG_MAX);
		fflush(stdout);

		if(pselect(sock + 1, &input, NULL, NULL, NULL, NULL) < 0){
			fprintf(stderr, "%s: Select error\n", PROGRAM_NAME);
			continue;
		}

		/* if the socket is active, print what's read from it.
		 * otherwise, read from stdin and send the msg to the sock
		 * if the first character of an inputted msg is :, 
		 * the user is attempting to execute a chat command (eg, quit)
		 * this message will not be send */ 
		if((FD_ISSET(sock, &input))){
			if(read_from_client(sock, msg, MSG_MAX) < 0)
				return;
			printf("%s", msg);
		} else if ((FD_ISSET(0, &input))){
			psgets(msg, MSG_MAX);
			if(msg[0] == ':'){
				if(chat_control(msg) == CHAT_DC){
					puts("Disconnecting...");
					shutdown(sock, SHUT_RDWR);
					return;
				}
			}
			else if(send_to_client(sock, uname, msg) < 0){
				return;
			}
		}
	}
}

/* trivial as-is, but could be expanded to offer more options */ 
int chat_control(char *msg){
	if(strncmp(msg, ":dc", 3) == 0) return -1;
	return 0;
}

int read_from_client(int sock, char *msg, size_t size){
	int bytesread;

	if((bytesread = read(sock, msg, size)) == 0){
		/* the connection has been terminated nicely by the client */
		printf("Remote connection closed...\n");
		shutdown(sock, SHUT_RDWR);
		return -1;
	} else if(bytesread < 0){
		/* the connection terminated abrupty (ie, read fails). 
		 * this could be made more robust */ 
		fprintf(stderr, "%s: Failed to recieve \
			message, terminating connection\n", 
			PROGRAM_NAME);
		shutdown(sock, SHUT_RDWR);
		return -1;
	}
	return 0;
}

int send_to_client(int sock, char *uname, char *msg){
	char send_msg[MSG_MAX];
	int nbytes;

	/* The username is appended to the beginning of each message
	 * this could be made more stateful by having an initial "handshake"
	 * that establishes usernames on both ends but oh well! */ 

	nbytes = snprintf(send_msg, MSG_MAX, "%s> %s", uname, msg);
	if(write(sock, send_msg, nbytes + 1) == -1){
		fprintf(stderr, "%s: Failed to send message\n",
				PROGRAM_NAME);
		printf("Remote host closed...\n");
		shutdown(sock, SHUT_RDWR);
		return -1;
	}
	return 0;
}	

int establish_client(int sock){
	struct sockaddr_in client;
	int clientsock;
	socklen_t size = sizeof(client);
	memset(&client, 0, sizeof(client));

	clientsock = accept(sock, (struct sockaddr*) &client, &size);
	if(clientsock < 0){
		fprintf(stderr, "%s: failed to accept\n", PROGRAM_NAME);
		return -1;
	}

	return clientsock;
}

int server_socket_setup(uint16_t *port){
	int sock;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock < 0){
		fprintf(stderr, "%s: Failed to create socket\n", PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(*port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
		fprintf(stderr, "%s: Failed to bind socket\n", PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}

	socklen_t len = sizeof(addr);
	getsockname(sock, (struct sockaddr *) &addr, &len);
	*port = ntohs(addr.sin_port);

	return sock;
}

int client_socket_setup(char *ip, uint16_t port){
	struct sockaddr_in client;
	int sock;
	memset(&client, 0, sizeof(client));

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		fprintf(stderr, "%s: Client socket error\n", PROGRAM_NAME);
		return -1;
	}

	client.sin_family = AF_INET;
	client.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &client.sin_addr) == 0){
		fprintf(stderr, "%s: Failed to parse IP\n", PROGRAM_NAME);
		return -1;
	}

	if(connect(sock, (struct sockaddr*) &client, sizeof(client)) < 0){
		fprintf(stderr, "%s: Failed to connect to server\n", PROGRAM_NAME);
		return -1;
	}
	
	return sock;
}

uint16_t  parse_port(char *portstr){
	/* a failure from strtol returns 0, failing the check implicitly */
	int port = strtol(portstr, NULL, 0);
	if(port > PORT_MAX || port < PORT_MIN){
		fprintf(stderr,"%s: Invalid port.\n", PROGRAM_NAME);
		return 0;
	}
	return (uint16_t) port;
}
