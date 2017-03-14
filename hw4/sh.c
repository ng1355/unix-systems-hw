/* Shell implimentation by Nikita Georgiou */

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_INPUT 1024 
#define IO_REFIR <>
#define IO_PIPE |

const char* PROGRAM_NAME;

void sh();
int exec_fork(char* name, char** arglist);
int redir(char* file1, char* flle2, char arrow);
size_t tokenize(char* input, char** arglist);

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];
	sh();
}

void printbuf(char** buf){
	puts("in printbuf");
	while(*buf) puts(*buf++);
}

void sh(){
	char input[MAX_INPUT];
	char* arglist[ARG_MAX];
	char* default_ps1 = ">>";
	char* ps1;

	if((ps1 = getenv("PS1")) != NULL) ps1 = default_ps1;

	while(1){
		printf("%s", ps1);
		if(fgets(input, MAX_INPUT, stdin) == NULL){
			perror(PROGRAM_NAME);
			continue;
		}

		if(tokenize(input, arglist) == 0) continue;

		if(exec_fork(input, arglist) == -1) continue;
	}

}

int exec_fork(char* name, char** arglist){
	pid_t pid = fork();

	if(pid== 0){
		execvp(arglist[0], &arglist[1]);
		fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, 
				name, strerror(errno));
		return -1;
	}
	else if(pid < 0){
		perror(PROGRAM_NAME);
		return -1;
	}
	wait(0);
	return 0;
}
/* IO redirection, redir is either < or > 
 * output < input reads input from file to prog's stdin
 * prog > file writes output from prog to file from stdout */
int redir(char* prog, char* filename, char arrow){
	FILE* file = fopen(filename, "w");
	if(arrow == '>'){
		dup2(1, fileno(file));
		exec_fork(prog, NULL);


size_t tokenize(char* input, char** arglist){
	size_t index = 0;
	char* token;

	if(input[0] == '\n') return 0;

	while((index < ARG_MAX) && (token = strtok_r(input, " \n", &input))){
		arglist[index++] = token;
	}
	
	arglist[index++] = (char*) 0;	
	return index;
}
