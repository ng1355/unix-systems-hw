//du implimentation by Nikita Georgiou
//Does not use ntfw
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

char* PROGRAM_NAME;

void du(char* dirname);

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
	struct dirent* currdir = readdir(dir);
	printf("currdir: %s\n", currdir->d_name);
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
