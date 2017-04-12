#include <stdio.h>
#include <sys/socket.h>

int main(){
	char ip[15], uname[16];
	int port;
	/* read username, ip, and port from user */
	config(uname, ip, port);
	/* set up socket to connect to server */ 
	connect();
	/* send msg to server from stdin, echo own msg & other msgs to stdout */
	communicate();
}

void config(char *uname, char *ip, int *port){
	printf("Enter username: ");
	fgets(uname, 16, stdin);
	printf("Enter host address: ");
	fgets(ip, 15, stdin);
        fgets	
}

void prompt(char *prompt, char *buf, int size){
	while(1){
		printf("%s", prompt);
		if(fgets(buf, size, stdin) == NULL){


