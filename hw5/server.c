#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT_MAX 65535
#define PORT_MIN 


int main(){
	int addr, port, sock;
	char *uname;
	/* configure settings from command line */ 
	config(&addr, &port, uname);
	/* set up sockety stuff */ 
	socket_setup();
	/* main loop, listen for a client, talk to them upon connection
	 * and resume listening if client dcs */ 
	while(!done){
		listen();
		done = chat();
	}
}

void config(char *addr, int *port, char *uname){
	char portstr[6];
	while(1){
		printf("Enter a username (16 chars max): ");
		if(psgets(uname, 16) < 0) continue;
		printf("Enter IP address; ");
		if(psgets(addr, 16) < 0) continue;
		printf("Enter port number: ");
		if(psgets(portstr, 5) < 0) continue;
	}
	port = strtol(portstr, NULL, 0);	
}

static inline int psgets(char *buf, size_t size){
	int n;
	char *readstr;
	readstr = fgets(buf, size, stdin);
	if(readstr == NULL){
		fprintf(stderr, "%s: Error reading from stdin.\n", 
				PROGRAM_NAME);
		return -1;
	}
	strtok(readstr, "\n");
	return strnlen(readstr, size) - 1;
}

static inline int validate_port(char *port){
	int portno = strtol(port, NULL, 0);
	if(portno < 1 || portno > 
