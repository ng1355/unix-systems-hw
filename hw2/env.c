#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char** environ;

#define FLAGSET (argc > 1 && !(strncmp(argv[1], "-i", 2)))

void env(int flag, char** argv);
int getflag(int argc, char** argv);
int getuserenv(int flag, char** argv, char** userenv);
char** concatEnviron(char** userenv, int userenv_size);

static inline int bufsize(char** buf);
static inline void printenv(char** userenv);

char** mallocmatrix(int row, int cols);
void printmatrix(char** matrix);


int main(int argc, char** argv){
	int flag = 0;

	if(FLAGSET){
		if(strncmp(argv[1], "-i", 3) != 0){
			fprintf(stderr, "Invalid argument\n");
			exit(EXIT_FAILURE);
		}
		flag = 1;
	}

	char** userenv = (char**)malloc(argc * sizeof(char*));
	int userenv_size = getuserenv(flag, argv, userenv);
	argv += userenv_size;

	userenv = (char**)realloc(userenv, userenv_size);
	if(!flag) userenv = concatEnviron(userenv, userenv_size);

	environ = userenv;

	if(bufsize(argv + flag) == 0){
		printenv(userenv);
		exit(EXIT_SUCCESS);
	}

	env(flag, argv);
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

static inline void printenv(char** userenv){
	while(*userenv) printf("%s\n", *userenv++);
}

char** concatEnviron(char** userenv, int userenv_size){
	char** newenviron = malloc((bufsize(environ) + userenv_size) * sizeof(char*));
	char** newenvironptr = newenviron;
	char** environptr = environ;
	while((*newenviron++ = *environptr++));
	newenviron--;
	while((*newenviron++ = *userenv++));

	return newenvironptr;
}

void env(int flag, char** argv){
	argv += flag;
	if(execvp(*argv, argv) == -1)
		printf("fail\n");
}

char** mallocmatrix(int rows, int cols){
	char** matrix = malloc(rows * sizeof(char));
	for(size_t i = 0; i < rows; i++)
		matrix[i] = malloc(cols * sizeof(char));
	return matrix;
}

void freematrix(char** matrix){
	int size = bufsize(matrix);
	for(size_t i = 0; i < size; i++)
		free(matrix[i]);
	free(matrix);
}

void printmatrix(char** matrix){
	while(*matrix) printf("%s\n", *matrix++);
}

static inline int bufsize(char** buf){
	int size = 0;
	while(*buf++) size++;
	return size;
}
