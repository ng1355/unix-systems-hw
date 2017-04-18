/* By Nikita Georgiou */ 

#include "common.h"

extern char *PROGRAM_NAME;

int psgets(char *buf, size_t size){
	char *readstr;
	readstr = fgets(buf, size, stdin);
	if(readstr == NULL){
		fprintf(stderr, "%s: Error reading from stdin.\n", 
				PROGRAM_NAME);
		return -1;
	}

	/* TODO: Reimpliment fpurge */ 
	return strnlen(readstr, size);
}

