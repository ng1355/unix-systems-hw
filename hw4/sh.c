/* Shell implimentation by Nikita Georgiou */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_STDINPUT 1024 

#ifndef ARG_MAX
#define ARG_MAX _POSIX_ARG_MAX
#endif

const char* PROGRAM_NAME;

void sh();
int exec_fork(char** arglist);
int redir(char** arglist, char* flle2, char arrow);
char** tokenize(char* input);
void cd(char* dir);

static inline void* smalloc(size_t size);
static inline void free_args(char** arglist);
static inline char* strstrchr(char* str, const char* delim);

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];
	sh();
}

void printbuf(char** buf){
	puts("in printbuf");
	while(*buf) puts(*buf++);
}

void sh(){
	char input[ARG_MAX];
	char** arglist = NULL;
	char* default_ps1 = ">>";
	char* ps1;

	/* any error here should be self-evident. Also it should be up
	 * to the user to decide if they want a trailing space or not */
	if((ps1 = getenv("PS1")) == NULL) ps1 = default_ps1;

	while(1){
		printf("%s", ps1);
		if(fgets(input, ARG_MAX, stdin) == NULL){
			perror(PROGRAM_NAME);
			continue;
		}

		if((arglist = tokenize(input)) == NULL) continue;

		if(strncmp(arglist[0], "cd", 2) == 0){ 
			cd(arglist[1]);
			free_args(arglist);
			continue;
		}
		else if(strncmp(arglist[0], "exit", 4) == 0)
			exit(EXIT_SUCCESS);

		exec_fork(arglist);
		free_args(arglist);
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
	else wait(0);
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

/* IO redirection, redir is either < or > 
 * output < input reads input from file to prog's stdin
 * prog > file writes output from prog to file from stdout */
int redir(char** arglist, char* filename, char arrow){
	FILE* file = fopen(filename, "w");
	pid_t pid = fork();

	if(pid == 0){
		if(arrow == '>'){
			dup2(1, fileno(file));
		}
		else if(arrow == '<'){
			dup2(0, fileno(file));
		}
		execvp(arglist[0], arglist);
	}
	else if(pid < 0){
		perror(PROGRAM_NAME);
		_Exit(EXIT_FAILURE);
	}
	else wait(0);
	return 0;
}

/* Splits single input string into array of strings delimited by space. 
 * also strips trailing newline appended by fgets. Appends null for exec. 
 * returns number of tokenized strings in input. */ 
char** tokenize(char* input){
	size_t index = 0, size = 10;
	char** arglist;
	char* token, *redirtok;

	if(input[0] == '\n') return NULL;

	if((arglist = malloc(size * sizeof(char*))) == NULL){
		perror(PROGRAM_NAME);
		return NULL;
	}

	while((token = strtok_r(input, " \n", &input))){
		if(index == size){
			if((realloc(arglist, size *= 2)) == NULL){
				perror(PROGRAM_NAME);
				return NULL;
			}
		}
		arglist[index++] = strdup(token);
	}
	if((redirtok = strchrchr(`
	arglist[index++] = (char*) 0;	
	return arglist;
}

static inline void* smalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return mem;
}

static inline void free_args(char** arglist){
	char** arglist_start = arglist;
	while(*arglist) free(*arglist++);
	free(arglist_start);
}

/* returns pointer to first instance of any of the chars in delim 
 * else returns null */ 
static inline char* strstrchr(char* str, const char* delim){
	while(*delim != '\0'){
		while(*str != '\0'){
			if(*str++ == *delim) return str;
		}
		delim++;
	}
	return NULL;
}
