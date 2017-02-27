//du implimentation by Nikita Georgiou
//Does not use ntfw
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char* PROGRAM_NAME;

int du(char* dirname);
void printdirs(DIR* dir);

static inline DIR* sopendir(char* dirname);
static inline char* getpath(char* path);

int main(int argc, char** argv) {
	PROGRAM_NAME = argv[0];

	if(argc == 1) du(".");
	else if(argc == 2) du(argv[1]);
	else{
		fprintf(stderr, "Usage: %s [dirname]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

int du(char* dirname) {
	DIR* dir = sopendir(dirname);
	struct dirent* currdir;
	struct stat* dirent_data = NULL;
	char* fullpath = NULL;
	int block_count = 0;
	//walk directory tree
	while((currdir = readdir(dir)) != NULL){
	//lstat dirent 
		fullpath = getpath(currdir->d_name);
//		printf("d_name: %s fullpath: %s\n", currdir->d_name, fullpath);
		if(lstat(currdir->d_name, dirent_data) != 0){ 
			puts("lstat error");
			perror(PROGRAM_NAME);
			continue;
		}
	//recurse if its a directory
		if(S_ISDIR(dirent_data->st_mode)) block_count += du(fullpath);
	//else accumulate blocksize
		else block_count += dirent_data->st_blksize;
	}
	//print directory size 
	printf("%d\t%s", block_count, fullpath);
	free(fullpath);
	closedir(dir);
	return block_count;
}

static inline DIR* sopendir(char* dirname){
	DIR* dir;
	if((dir = opendir(dirname)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return dir;
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
