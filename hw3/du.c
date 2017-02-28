//du implimentation by Nikita Georgiou
//Does not use ntfw
//ran out of time teehee
//i just did so much commenting omg, hardlink counts work!!!!!
//TODO: cleanup 

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

/* a simple vector to record encountered inodes
 * will be used more like a map in implimentation.
 * As such, there's no check for it, but its assumed each unique inode
 * will have only one entry. */
typedef struct{
	ino_t* inodes;
	size_t capacity;
	size_t size;
} inode_list;

int du(char* fullpath, inode_list* inodes);

int inode_exists(ino_t inode, ino_t* inodes);
inode_list* new_inodelist(size_t capacity, size_t size);
size_t count_inode(ino_t inode, inode_list* inodes);
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
 * Returns number of blocks in a given directory for recusion. 
 * TODO: cleanup, stack memory safety. :( */
int du(char* basepath, inode_list* inodes){
	DIR* dir;
	struct dirent* entry;
	struct stat entdata;
	char new_path[PATH_MAX];
	char* dname;
	long int blockcount = 0;
	size_t inode_exists = 0;

	if((dir = opendir(basepath)) == NULL){
			perror(PROGRAM_NAME);
			return 0;
	}

	while((entry = readdir(dir)) != NULL){
		dname = entry->d_name;

		//Make sure dirent's not ".." 
		if(((strnlen(dname, 3) == 2) && (strncmp(dname, "..", 2) == 0)))
			continue;

		/*If there's a record of the inode in the inode list, 
		 * skip the loop. Otherwise, add it to the list if it's not 
		 * already there. */ 
		if((inode_exists = count_inode(entry->d_ino, inodes)) != 0)
			continue;

		if(inode_exists != 1) add_inode(entry->d_ino, inodes);

		//Append files/directory name to provided dir
		//TODO: Some absolute paths Have extraneous "/" at beginning
		//harmless, but not visually pleasing
		sprintf(new_path, "%s/%s", basepath, dname);

		if(lstat(new_path, &entdata) != 0){
			perror(PROGRAM_NAME);
			continue;
		}

		//if its a directory but not ".", recurse into it
		//giving the folder relative to the cwd.
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

//mallocs or dies. im getting lazier with the names lol
static inline void* smalloc(size_t size){
	void* mem;
	if((mem = malloc(size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return mem;
}

//Essentially a constructor for the inode list
//dynamic array for to store inode numbers, as well as struct itself
//returns pointer to inode list struct 
inode_list* new_inodelist(size_t capacity, size_t size){
	inode_list* inodes;
	inodes = (inode_list*) smalloc(sizeof(inode_list));
	inodes->inodes = (ino_t*) smalloc(capacity * sizeof(ino_t));
	inodes->capacity = capacity;
	inodes->size = size;
	return inodes;
}

//Counts the number of inode numbers matching "inode", shouldnt be more than 1
//returns that count
size_t count_inode(ino_t inode, inode_list* inodes){
	size_t count = 0;
	for(size_t i = 0; i < inodes->size; i++)
		if(inodes->inodes[i] == inode) count++;
	return count;
}

//Adds a new inode to the inode list's list. If the list runs out of space,
//double capacity a la vector (well, not quite) with realloc. 
void add_inode(ino_t inode, inode_list* inodes){
	if(inodes->size == inodes->capacity){
		inodes->inodes = srealloc(inodes->inodes, sizeof(ino_t) * inodes->capacity * 2);
		inodes->capacity *= 2;
	}
	inodes->inodes[inodes->size += 1] = inode;
}

//destructor 
void delete_inodelist(inode_list* inodes){
	free(inodes->inodes);
	free(inodes);
}

//reallocs or dies trying 
static inline void* srealloc(void* ptr, size_t size){
	if((ptr = realloc(ptr, size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return ptr;
}
