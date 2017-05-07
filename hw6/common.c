/* By Nikita Georgiou */ 

#include "common.h"

extern char *PROGRAM_NAME;

int psgets(char *str, size_t size){
	int nbytes;
	if((nbytes = read(0, str, size)) == -1){
		fprintf(stderr, "%s: Read error\n", PROGRAM_NAME);
		return -1;
	}
	str[nbytes - 1] = '\0';
	return nbytes - 1;
}

