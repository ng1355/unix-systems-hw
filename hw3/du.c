//du implimentation by Nikita Georgiou
//Does not use ntfw
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

char* PROGRAM_NAME;

void du(char* dirname);
void printdirs(DIR* dir);

static inline DIR* sopendir(char* dirname);

int main(int argc, char** argv) {
	PROGRAM_NAME = argv[0];

	if(argc == 1) du(".");
	else if(argc == 2) du(argv[1]);
	else{
		fprintf(stderr, "Usage: %s [dirname]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

void du(char* dirname) {
	DIR* dir = sopendir(dirname);
	printdirs(dir);
	closedir(dir);
}

static inline DIR* sopendir(char* dirname){
	DIR* dir;
	if((dir = opendir(dirname)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return dir;
}

void printdirs(DIR* dir){
	struct dirent* currdir;
	struct stat* buf;
	while((currdir = readdir(dir)) != NULL){

}

/* gets canonical path of filename
 * As of POSIX2008, passing NULL to realpath means path will point to a block
 * of memory at least PATH_MAX in size, older standard have implimentation
 * specific effects. Returns a pointer to the malloced path */
static inline char* getpath(char* filename){
	char* path;
	if((path = realpath(filename, NULL)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return path;
}
