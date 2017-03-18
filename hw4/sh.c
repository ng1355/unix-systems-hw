/* Shell implimentation by Nikita Georgiou 
 * TODO: eliminate  redundancy, cleanup spaghett, impliment redir chaining */ 

#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

/* ARG_MAX is interestingly not defined on Windows' Linux subsystem */ 
#ifndef ARG_MAX
#define ARG_MAX _POSIX_ARG_MAX
#endif

const char* PROGRAM_NAME;

/* checks if two strings are the same, including their length */ 
#define STREQ(str1, str2, max) (!(strncmp((str1), (str2), (max))) && \
				(strnlen((str1), (max) + 1) == strnlen((str2),\
				(max) + 1)))

void sh();
void exec_fork(char** arglist);
void redir(char** arglist, char* flle2, char* arrow);
void redir_fork(char** arglist, char* flle2, char* arrow);
char** tokenize(char* input);
void cd(char* dir);
void eval_args(char** arglist);
char** fmt_redir(char** arglist, char** redir, size_t size);

static inline void printerr(char* failed_arg);
static inline char chrchr(char* str, const char* delim);
static inline size_t bufsize(char** buf);

int main(int argc, char** argv){
	PROGRAM_NAME = argv[0];
	sh();
}

/* Interactive part of the interative shell. Driver function for eval_args, 
 * handles some edge cases (empty user input and shell builtin functions) */
void sh(){
	char input[ARG_MAX];
	char** arglist = NULL;
	char* default_ps1 = ">>";
	char* ps1;

	/* any error here should be self-evident. */ 
	if((ps1 = getenv("PS1")) == NULL) ps1 = default_ps1;

	while(1){
		printf("%s", ps1);
		if(fgets(input, ARG_MAX, stdin) == NULL){
			perror(PROGRAM_NAME);
			continue;
		}

		if((arglist = tokenize(input)) == NULL) continue;

		if(STREQ(arglist[0], "cd", 2)){ 
			(bufsize(arglist) > 1) ? cd(arglist[1]) : cd("~");
			free(arglist);
			continue;
		}
		else if(STREQ(arglist[0], "exit", 4)){
			free(arglist);
			exit(EXIT_SUCCESS);
		}

		eval_args(arglist);
		free(arglist);
	}

}

/* fork wrapper, execs if its a child, _exit-ing on exec failure
 * if parent, waits on child. */ 
void exec_fork(char** arglist){
	pid_t pid = fork();

	if(pid == 0){
		execvp(arglist[0], arglist);
		printerr("exec");
		_exit(EXIT_FAILURE);
	}
	else if(pid < 0){
		printerr("fork");
		return;
	}
	else wait(0);
}

/* iterates over arglist tokenized by sh() and scans for redirection symbols
 * a token is assumed to be immediately followed by a filename. */
void eval_args(char** arglist){
	char** start = arglist, **newlist;
	char redirtok;

	while(*arglist){
		if((redirtok = chrchr(*arglist, "<>")) != '\0'){
			if(*(arglist + 1) == (char*) NULL){
				fprintf(stderr, "%s: syntax error near token"
					" %s\n", PROGRAM_NAME, *arglist);
				return;
			}
			if((newlist = 
			   fmt_redir(start, arglist, bufsize(start))) == NULL){
				printerr("malloc");
				return;
			}
			redir_fork(newlist, *(arglist + 1), *arglist);
			free(newlist);
			*arglist = (char*) NULL;
			start = ++arglist;
		}
		else arglist++;
	}
	exec_fork(start);
}

/* mallocs a new char** that holds all non-redirection tokens from arglist
 * this is used rearrange oddly placed tokens into a format redir() understands
 */
char** fmt_redir(char** arglist, char** redir,  size_t size){
	char** newlist, **newlist_start;
	if((newlist = malloc((size - 1) * sizeof(char*))) == NULL)
		return (char**) NULL;
	newlist_start = newlist;
	while(*arglist){
		if((*arglist == *redir) || (*arglist == *(redir + 1))){
			arglist++;
			continue;
		}
		*(newlist++) = *(arglist++);
	}
	*newlist = *arglist;
	return newlist_start;
}

/* driver function for redir, does the same thing as exec_fork but assumes 
 * redirection will be performed, and calls redir() in the child */
void redir_fork(char** arglist, char* filename, char* arrow){
	pid_t pid = fork();

	if(pid == 0){
		redir(arglist, filename, arrow);
		_exit(EXIT_FAILURE);
	}
	else if(pid < 0){
		printerr("fork");
		return;
	}
	else wait(0);
}

/* performs io redirection by either > < or >>. tokens can be preceded by 
 * a number specifying which descriptor to read/write from. fork()s and replaces
 * child with program being run, using dup2 to control what happens to stdin 
 * and out. returns on failure. */
void redir(char** arglist, char* filename, char* arrow){
	FILE* file;
	int fd;
	char* endptr, *mode;
	/* interestingly, the number before a token can be used to
	 * specify an arbitrary file descriptor */ 
	if((fd = strtol(arrow, &endptr, 0)) == 0)
		fd = (endptr[0] == '<') ? 0 : 1;
	else if(fd < 0){
		fprintf(stderr, "%s: bad fd for redirection\n", 
				PROGRAM_NAME);
		return;
	}

	if(strncmp(endptr, ">>", 2) == 0) mode = "a";
	else mode = (endptr[0] == '<') ? "r" : "w"; //possibly redundant

	if((file = fopen(filename, mode)) == NULL){
		printerr(filename);
		return;
	}

	dup2(fileno(file), fd);
	execvp(arglist[0], arglist);
	fclose(file);
}

/* Splits single input string into array of strings delimited by space. 
 * also strips trailing newline appended by fgets. Appends null for exec. 
 * returns address to malloced array */ 
char** tokenize(char* input){
	size_t index = 0, size = 10;
	char** arglist;
	char* token;

	if(input[0] == '\n') return NULL;

	if((arglist = malloc(size * sizeof(char*))) == NULL){
		perror(PROGRAM_NAME);
		return NULL;
	}

	while((token = strtok_r(input, " \n", &input))){
		if(index == size){
			if((realloc(arglist, size *= 2)) == NULL){
				perror(PROGRAM_NAME);
				free(arglist);
				return NULL;
			}
		}
		arglist[index++] = token;
	}

	arglist[index++] = (char*) 0;	
	return arglist;
}

/* changes shell's cwd to dir or prints error and does notthing. 
 * A successful directory change will attempt to create or set the pwd/oldpwd
 * env vars, but there is no guarintee setenv will succeed
 * and no check for its failure */ 
void cd(char* dir){
	/* conflicting documentation: Darwin's manpage says realpath uses
	 * PATH_MAX as a lower limit, but man7 and manpages on my linux distro
	 * say its an upper limit. */ 
	char pwd[PATH_MAX];
	char oldpwd[PATH_MAX];
	char* pwd_ptr, *oldpwd_ptr;

	//need to do this or Werror yells about unused returns
	oldpwd_ptr = realpath(".", oldpwd);

	if(STREQ(dir, "~", 1)){
		if((pwd_ptr = getenv("HOME")) == NULL) return;
	}
	else if((pwd_ptr = realpath(dir, pwd)) == NULL){
		printerr(dir);
		return;
	}

	if(chdir(pwd_ptr) == -1){
		printerr(dir);
		return;
	}
	
	if(oldpwd_ptr != NULL) setenv("OLDPWD", oldpwd, 1);
	if(pwd_ptr != NULL) setenv("PWD", pwd, 1);
}

/* nicer than perror */ 
static inline void printerr(char* failed_arg){
	fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, failed_arg,
			strerror(errno));
}

/* checks if the trailing non-null character of string str is any char in delim
 * returns first char that matches, else returns null */ 
static inline char chrchr(char* str, const char* delim){
	size_t size = strlen(str); //replace with strnlen
	while(*delim){
		if(str[size - 1] == *delim) return str[size - 1];
		delim++;
	}
	return '\0';
}

//gets number of items pointed by char** buf
static inline size_t bufsize(char** buf){
	size_t size = 0;
	while(*buf++) size++;
	return size;
}
