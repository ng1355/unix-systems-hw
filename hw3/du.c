//du implimentation by Nikita Georgiou
//Does not use ntfw
//ran out of time teehee
//TODO: Impliment inode counting for hardlinks
//Document my freaking VECTOR & actually make it work lol

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

const char* PROGRAM_NAME; //For error printing

typedef struct{
	ino_t* inodes;
	size_t capacity;
	size_t size;
} inode_list;

int du(char* fullpath, inode_list* inodes);

int inode_exists(ino_t inode, ino_t* inodes);
inode_list* new_inodelist(size_t capacity, size_t size);
size_t find_inode(ino_t inode, inode_list* inodes);
void add_inode(ino_t inode, inode_list* inodes);
void delete_inodelist(inode_list* inodes);


static inline void* smalloc(size_t size);
static inline void* srealloc(void* ptr, size_t size);


int main(int argc, char** argv) {
	PROGRAM_NAME = argv[0];
	
	//Setting up inode vector to keep track of hardlinks
	inode_list* inodes;
	inodes = new_inodelist(10, 0);

	if(argc == 1) du(".", inodes);
	else if(argc == 2) du(argv[1], inodes);
	else{
		fprintf(stderr, "Usage: %s [dirname]\n", argv[0]);
		free(inodes);
		exit(EXIT_FAILURE);
	}
	delete_inodelist(inodes);
}

/* Traverses the directory tree and sums the how many 1024B blocks each file
 * takes up. basepath can be either relative or absolute, and is at most 
 * PATH_MAX. Recurses into subdirectories  to sum their disk usage. 
 * Prints directories that have been summed and their corresponding size. 
 * Returns number of blocks in a given directory. 
 * TODO: cleanup, memory safety. :( */
int du(char* basepath, inode_list* inodes){
	DIR* dir;
	struct dirent* entry;
	struct stat entdata;
	char new_path[PATH_MAX];
	char* dname;
	long int blockcount = 0;

	if((dir = opendir(basepath)) == NULL){
			fprintf(stderr, "%s: cannot read directory '%s'",
				PROGRAM_NAME, basepath);
			perror("");
			return 0;
	}

	while((entry = readdir(dir)) != NULL){
		dname = entry->d_name;

		//Make sure dirent's not .. 
		if(((strnlen(dname, 3) == 2) && (strncmp(dname, "..", 2) == 0)))
			continue;

		if(find_inode(entry->d_ino, inodes) != 0){ 
			add_inode(entry->d_ino, inodes);
			continue;
		}

		//Append files/directory name to provided dir
		sprintf(new_path, "%s/%s", basepath, dname);

		if(lstat(new_path, &entdata) != 0){
			fprintf(stderr, "%s: cannot read directory '%s'",
				PROGRAM_NAME, new_path);
			perror("");
			continue;
		}

		if(S_ISDIR(entdata.st_mode)){
			if(((strnlen(dname, 2) == 1) && (dname[0] == '.')))
				blockcount += entdata.st_blocks / 2;
			else
				blockcount += du(new_path, inodes);
		}
		else blockcount += entdata.st_blocks / 2;
	}

	printf("%ld\t%s\n", blockcount, basepath);
	closedir(dir);
	return blockcount;
}

static inline void* smalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return mem;
}

inode_list* new_inodelist(size_t capacity, size_t size){
	inode_list* inodes;
	inodes = (inode_list*) smalloc(sizeof(inode_list));
	inodes->inodes = (ino_t*) smalloc(capacity * sizeof(ino_t));
	inodes->capacity = capacity;
	inodes->size = size;
	return inodes;
}

size_t find_inode(ino_t inode, inode_list* inodes){
	size_t loc = 0;
	for(size_t i = 0; i < inodes->size; i++)
		if(inodes->inodes[i] == inode){
			loc = i;
			break;
		}
	return loc;
}

void add_inode(ino_t inode, inode_list* inodes){
	if(inodes->size == inodes->capacity){
		inodes->inodes = srealloc(inodes->inodes, inodes->capacity * 2);
		inodes->capacity *= 2;
	}

	inodes->inodes[inodes->size + 1] = inode;
	inodes->size++;
}

void delete_inodelist(inode_list* inodes){
	free(inodes->inodes);
	free(inodes);
}

static inline void* srealloc(void* ptr, size_t size){
	if((ptr = realloc(ptr, size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

