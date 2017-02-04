#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char** environ;

#define FLAGSET (argc > 1 && !(strncmp(argv[1], "-i", 2)))
#define NEWENV_SIZE ((bufsize(environ) + userenv_size) * sizeof(char*))

int getflag(int argc, char** argv);
int getuserenv(int flag, char** argv, char** userenv);
char** concatEnviron(char** userenv, int userenv_size);

static inline void env(int flag, char** argv);
static inline void* safemalloc(size_t size);
static inline void freematrix(char** matrix);
static inline int bufsize(char** buf);
static inline void printenv(char** userenv);

int main(int argc, char** argv){
	int flag = getflag(argc, argv);

	char** userenv = (char**)safemalloc(argc * sizeof(char*));
	int userenv_size = getuserenv(flag, argv, userenv);
	argv += userenv_size;

	if(!flag) userenv = concatEnviron(userenv, userenv_size);

	if(bufsize(argv + flag) == 0){
		printenv(userenv);
		freematrix(userenv);
		exit(EXIT_SUCCESS);
	}

	environ = userenv;
	env(flag, argv);
}

static inline void env(int flag, char** argv){
	argv += flag;
	execvp(*argv, argv);

	perror("env");
	exit(EXIT_FAILURE);
}

int getflag(int argc, char** argv){
	int flag = 0;
	if(FLAGSET){
		if(strncmp(argv[1], "-i", 3) != 0){
			fprintf(stderr, "env: invalid argument\n");
			exit(EXIT_FAILURE);
		}
		flag = 1;
	}
	return flag;
}

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

char** concatEnviron(char** userenv, int userenv_size){
	char** newenviron = safemalloc(NEWENV_SIZE);
	char** newenvironptr = newenviron;
	char** environptr = environ;
	char** userenv_start = userenv;

	while((*newenviron++ = *environptr++));
	newenviron--;
	while((*newenviron++ = *userenv++));

	freematrix(userenv_start);
	return newenvironptr;
}

static inline void freematrix(char** matrix){
	char** matrix_start = matrix;
	while(*matrix) free(*matrix++);
	free(matrix_start);
}


static inline void printenv(char** userenv){
	while(*userenv) printf("%s\n", *userenv++);
}

static inline void* safemalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror("Error");
		exit(EXIT_FAILURE);
	}
	return mem;
}

static inline int bufsize(char** buf){
	int size = 0;
	while(*buf++) size++;
	return size;
}
