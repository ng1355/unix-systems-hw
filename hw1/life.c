#include <time.h>
#include <stdio.h>
#include <sys/mmap.h>
#include <fcntl.h>
#include <stdlib.h>

struct tile{
	char state =  ' ';

int main(int argc, char** argv){
	int fd;
	unsigned rows = 10, cols = 10, generations = 10;
	char* filename = "file.txt";
	//elif tree for command line args
	switch(argc){
		case 5: generations = argv[5];
		case 4: filename = argv[4];
		case 3: columns = argv[3];
		case 2: rows = argv[2];
		case 1: break;
		default: 
			fprintf(2, "Usage: life rows columns filename generations\n");
			
	//open file, check if we opened correctly
	if((fd = open(filename)) == -1){
		fprintf(2, "Error: couldn't open file\n");
		exit(0);
	}
	
	//put file in memory
	mmap(

}

void tick(){
}


