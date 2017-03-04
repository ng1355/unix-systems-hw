/* Shell implimentation by Nikita Georgiou */

#include <stdio.h>

int main(){
	sh();
}

void sh(){
	char arglist [ARG_MAX];
	if(fgets(arglist, ARG_MAX - 1, stdin) == NULL
}

