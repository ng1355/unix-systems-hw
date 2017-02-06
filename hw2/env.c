//env implimentation by Nikita Georgiou 
//going for more C-lookin naming conventions and trippy arithmetic

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char** environ;

//NEWENV_SIZE is the size of the buffer that holds both the 
//environ and user provided enviroment vars
#define NEWENV_SIZE ((bufsize(environ) + userenv_size) * sizeof(char*))

int getflag(int argc, char** argv);
int getuserenv(int flag, char** argv, char** userenv);
char** concatEnviron(char** userenv, int userenv_size);

static inline void env(int flag, char** argv);
static inline void* safemalloc(size_t size);
static inline int bufsize(char** buf);
static inline void printenv(char** userenv);

int main(int argc, char** argv){
	//check if -i is set
	int flag = getflag(argc, argv);

	//userenv will store pointers to user assigned environ vars
	//the number of user envs is guarinteed < argc so mallocing
	//this much is a safe assumption. 
	char** userenv = (char**)safemalloc(argc * sizeof(char*));
	
	//getuserenv will extract any such assignments from argv, the size is
	//used to offset argv so that it points after any assignments
	int userenv_size = getuserenv(flag, argv, userenv);
	argv += userenv_size;

	//concatinates the environ list to the user provided list if the -i
	//flag is omitted
	if(!flag) userenv = concatEnviron(userenv, userenv_size);

	//if no arguments follow env assignments, free resources and print
	//the enviroment
	if(bufsize(argv + flag) == 0){
		printenv(userenv);
		free(userenv);
		exit(EXIT_SUCCESS);
	}

	//Since execvpe isnt portable, environ needs to be reassigned so 
	//execvp can read from it. 
	environ = userenv;
	env(flag, argv);

	//Cleanup environ (points to malloced userenv) and terminate
	//incase execvp fails
	free(environ);
	exit(EXIT_FAILURE);

}

/* Sets argv to the first arg following user set env vars
 * first element of argv is the program to execute as well as a pointer
 * to the rest of the arguments for exec
 * exec reclaims all malloced resources so no need to free anything
 * If execvp fails, program prints error and returns. */
static inline void env(int flag, char** argv){
	argv += flag;
	execvp(*argv, argv);
	perror("env: exec");
}

/* checks if the first argument is the -i flag 
 * note that the flag's value will be used to offset the argv pointer
 * aborts program if any other kind of flag is provided */
int getflag(int argc, char** argv){
	int flag = 0;
	if(argc > 1 && argv[1][0] == '-'){
		if(strncmp(argv[1], "-i\0", 3) != 0){
			fprintf(stderr, "env: invalid argument\n");
			exit(EXIT_FAILURE);
		}
		flag = 1;
	}
	return flag;
}

/* scans argv for all assignments ("=") and places ther pointer into 
 * the userenv buffer
 * returns number of assignments found
 * assumes all assignments come before exec arguments and after -i*/
int getuserenv(int flag, char** argv, char** userenv){
	int size = 0;
	argv += flag + 1;
	while(*argv && strchr(*argv, '=')){
		*userenv++ = *argv++;
		++size;
	}
	*userenv = NULL; ++size;
	return size;	
}

/* Used when the -i flag isnt set, creates a new buffer of the combined
 * sizes of environ and userenv, and copies the char*s of both into the
 * new buffer. 
 * returns a pointer to the beginning of the buffer 
 * TODO: find a portable way to extract the size value malloc and free use
 * so i dont have to make so many tracking variables... */
char** concatEnviron(char** userenv, int userenv_size){
	char** newenviron = (char**)safemalloc(NEWENV_SIZE);
	char** newenviron_start = newenviron;
	char** environptr = environ;
	char** olduserenv = userenv;

	while((*newenviron++ = *environptr++));
	//realigns pointer since the while loop copies environ's trailing NULL
	newenviron--; 
	while((*newenviron++ = *userenv++));

	//frees old userenv buffer
	free(olduserenv);
	return newenviron_start;
}

//prints every string pointed to by the char** buffer
//buffer must be null terminated
static inline void printenv(char** userenv){
	while(*userenv) printf("%s\n", *userenv++);
}

//mallocs size bytes or dies trying
//returns pointer to start of buffer
static inline void* safemalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror("env error");
		exit(EXIT_FAILURE);
	}
	return mem;
}

//returns number of char*s in a char** buffer
static inline int bufsize(char** buf){
	int size = 0;
	while(*buf++) size++;
	return size;
}
