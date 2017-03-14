/* Shell implimentation by Nikita Georgiou */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdlib.h>

#define MAX_STDINPUT 1024 

#ifndef ARG_MAX
#define ARG_MAX _POSIX_ARG_MAX
#endif

const char* PROGRAM_NAME;

void sh();
int exec_fork(char** arglist);
int redir(char* file1, char* flle2, char arrow);
size_t tokenize(char* input, char** arglist);
void cd(char* dir);

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];
	sh();
}

void printbuf(char** buf){
	puts("in printbuf");
	while(*buf) puts(*buf++);
}

void sh(){
	char input[MAX_STDINPUT];
	char* arglist[ARG_MAX];
	char* default_ps1 = ">>";
	char* ps1;

	/* any error here should be self-evident. Also it should be up
	 * to the user to decide if they want a trailing space or not */
	if((ps1 = getenv("PS1")) == NULL) ps1 = default_ps1;

	while(1){
		printf("%s", ps1);
		if(fgets(input, MAX_STDINPUT, stdin) == NULL){
			perror(PROGRAM_NAME);
			continue;
		}

		if(tokenize(input, arglist) == 0) continue;

		if(strncmp(arglist[0], "cd", 2) == 0){ 
			cd(arglist[1]);
			continue;
		}
		else if(strncmp(arglist[0], "exit", 4) == 0)
			exit(EXIT_SUCCESS);

		/* if((strchr(input, '>') != NULL) || (strchr(input, '<') != NULL)){
			redir(
			*/
		if(exec_fork(arglist) == -1) continue;
	}

}

/* forks a child process specified by the first string in arglist
 * returns an error if execvp fails (eg program specified doesnt exist)
 * or fork fails. waits for child to terminate before returning successfully. */
int exec_fork(char** arglist){
	pid_t pid = fork();

	if(pid == 0){
		execvp(arglist[0], &arglist[0]);
		fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, 
				arglist[0], strerror(errno));
		_exit(EXIT_FAILURE);
	}
	else if(pid < 0){
		perror(PROGRAM_NAME);
		return -1;
	}
	wait(0);
	return 0;
}

/* changes shell's cwd to dir or prints error and does notthing. 
 * A successful directory change will attempt to create or set the pwd/oldpwd
 * env vars, but there is no guarintee setenv or realpath will succeed
 * and no check for their failure */ 
void cd(char* dir){
	char pwd[PATH_MAX];
	char oldpwd[PATH_MAX];
	char* pwd_ptr, *oldpwd_ptr;

	//need to do this or Werror yells about unused returns
	oldpwd_ptr = realpath(".", oldpwd);
	pwd_ptr = realpath(dir, pwd);

	if(chdir(dir) == -1){
		perror(PROGRAM_NAME);
		return;
	}
	
	if(oldpwd_ptr != NULL) setenv("OLDPWD", oldpwd, 1);
	if(pwd_ptr != NULL) setenv("PWD", pwd, 1);
}

void eval_args(char** arglist){
i	while(*args){
		if(strcmp	

/* IO redirection, redir is either < or > 
 * output < input reads input from file to prog's stdin
 * prog > file writes output from prog to file from stdout */
int redir(char* prog, char* filename, char arrow){
	FILE* file = fopen(filename, "w");
	if(arrow == '>'){
		dup2(1, fileno(file));
//		exec_fork(prog);
	}
	else if(arrow == '<'){
		dup2(0, fileno(file));
//		exec_fork(prog)
	}
	return 0;
}

/* Splits single input string into array of strings delimited by space. 
 * also strips trailing newline appended by fgets. Appends null for exec. 
 * returns number of tokenized strings in input. */ 
size_t tokenize(char* input, char** arglist){
	size_t index = 0;
	char* token, *redir;

	if(input[0] == '\n') return 0;

	while((index < ARG_MAX) && (token = strtok_r(input, " \n", &input))){
		arglist[index++] = token;
	}
	
	arglist[index++] = (char*) 0;	
	return index;
}
