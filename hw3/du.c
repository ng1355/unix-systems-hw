//du implimentation by Nikita Georgiou
//Does not use ntfw

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

//meant to determine if the length of str1 + str2 exceeds max (ie PATH_MAX)
#define IS_OVERFLOW(str1, str2, max) (((str1) + (str2)) > (max))

//common comparison that makes ifs long. Checks if dirent's d_name is .
#define IS_DOTDIR(d_name) ((strnlen((d_name), 2) == 1) && ((d_name)[0] == '.'))

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

inode_list* new_inodelist(size_t capacity);
int inode_exists(ino_t inode, inode_list* inodes);
void add_inode(ino_t inode, inode_list* inodes);
void delete_inodelist(inode_list* inodes);

static inline void* smalloc(size_t size);
static inline void* srealloc(void* ptr, size_t size);
static inline void* scalloc(size_t nemb, size_t size);
static inline void zero_out(ino_t* inodes, size_t start, size_t end);

int main(int argc, char** argv) {
	PROGRAM_NAME = argv[0];
	
	//Setting up inode vector to keep track of hardlinks
	inode_list* inodes = new_inodelist(10);

	if(argc == 1) du(".", inodes);
	else if(argc == 2){
		if(strnlen(argv[1], PATH_MAX) + 1 > PATH_MAX){
			fprintf(stderr, "%s: path too long\n", PROGRAM_NAME);
			exit(EXIT_FAILURE);
		}
	       	du(argv[1], inodes);
	}
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
 * Returns number of blocks in a given directory for recusion. */
int du(char* basepath, inode_list* inodes){
	DIR* dir;
	struct dirent* entry;
	struct stat entdata;
	char new_path[PATH_MAX];
	char* dname;
	long int blockcount = 0;

	if((dir = opendir(basepath)) == NULL){
		fprintf(stderr, "%s: couldn't open directory: '%s'; %s\n",
			PROGRAM_NAME, basepath, strerror(errno));
		return 0;
	}

	while((entry = readdir(dir)) != NULL){
		/* dont waste time with directories that exceed PATH_MAX
		 * as to avoid smashing new_path 
		 * doesnt abort like in main */
		if(IS_OVERFLOW(strnlen(basepath, PATH_MAX) + 1, 
				strnlen(basepath, PATH_MAX) + 1, PATH_MAX)){
			fprintf(stderr, "%s: path too long\n", PROGRAM_NAME);
			continue;
		}

		dname = entry->d_name;

		//Make sure dirent's not ".." 
		if(((strnlen(dname, 3) == 2) && (strncmp(dname, "..", 2) == 0)))
			continue;

		/* If there's a record of the inode in the inode list, 
		 * skip the loop. Otherwise, add it to the list if it's not 
		 * already there. First check is for case where dname
		 * is "." and must be counted but not checked against
		 * as this dir's inode is added to the list *before*
		 * it is recursed into, but doesn't have its size
		 * (the dir, not its contents) summed until now. */
		if(!(IS_DOTDIR(dname))){
			if(inode_exists(entry->d_ino, inodes)) continue;
			else add_inode(entry->d_ino, inodes);
		}

		//Append files/directory name to provided dir
		//check necessary for edgecase where basepath is root ('/')
		//else "%s/%s" adds extraneous / to new_path
		if((strnlen(basepath, 2) == 1) && (basepath[0] == '/'))
			snprintf(new_path, PATH_MAX, "%s%s", basepath, dname);
		else 
			snprintf(new_path, PATH_MAX, "%s/%s", basepath, dname);

		if(lstat(new_path, &entdata) != 0){
			fprintf(stderr, "%s: couldn't stat path: %s\n",
				PROGRAM_NAME, new_path);
			continue;
		}

		//if its a directory but not ".", recurse into it
		//giving the folder relative to the cwd.
		if(S_ISDIR(entdata.st_mode)){
			if(IS_DOTDIR(dname))
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

/* Essentially a constructor for the inode list
 * dynamic array for to store inode numbers, as well as struct itself
 * note that the dynamic array is calloced, and therefor zeroed out. 
 * returns pointer to inode list struct */
inode_list* new_inodelist(size_t capacity){
	inode_list* inodes;
	inodes = smalloc(sizeof(inode_list));
	inodes->inodes = scalloc(sizeof(ino_t), capacity);
	inodes->capacity = capacity;
	inodes->size = 0;
	return inodes;
}

//Counts the number of inode numbers matching "inode", shouldnt be more than 1
//returns that count
int inode_exists(ino_t inode, inode_list* inodes){
	for(size_t i = 0; i < inodes->size; i++)
		if(inodes->inodes[i] == inode) return 1;
	return 0;
}

//Adds a new inode to the inode list's list. If the list runs out of space,
//increase capacity a la vector with realloc. 
void add_inode(ino_t inode, inode_list* inodes){
	if(inodes->size == inodes->capacity){
		inodes->inodes = srealloc(inodes->inodes, 
				 sizeof(ino_t) * inodes->capacity * 2);
		inodes->capacity *= 2;
		zero_out(inodes->inodes, inodes->size, inodes->capacity);
	}
	inodes->inodes[inodes->size++] = inode;
}

//destructor 
void delete_inodelist(inode_list* inodes){
	free(inodes->inodes);
	free(inodes);
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

//reallocs or dies trying 
static inline void* srealloc(void* ptr, size_t size){
	if((ptr = realloc(ptr, size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return ptr;
}

/* callocs or dies trying. Note that this nemb & size cannot be zero */
static inline void* scalloc(size_t nemb, size_t size){
	void* mem;
	if((mem = calloc(nemb, size)) == NULL){
		perror(PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}
	return mem;
}

static inline void zero_out(ino_t* inodes, size_t start, size_t end){
	while(start != end) inodes[start++] = (ino_t) 0;
}
