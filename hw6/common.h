/* Useful functions for homework by Nikita Georgiou  */ 

#ifndef _NIKITA_COMMON_H
#define _NIKITA_COMMON_H

#include <stdio.h> 
#include <string.h>
#include <unistd.h> 
#include <poll.h>

#define NL_PAD 2 /* padding for bufs from stdin, accomodates newline and \0 */

/* prints program name & corresponding error. Terminates the program. */ 
#define eperror() { perror(PROGRAM_NAME); exit(EXIT_FAILURE); }

/* Reads input from stdin into buf of up to size bytes. Input longer
 * than size is truncated to size - 1 (excluding null terminator).
 * Returns the number of bytes read, or -1 on error. Also prints error msg
 * on error. */ 
int psgets(char*, size_t);

/* polls on fd fdno, consuming any remaining characters in the file descriptor
 * this in effect purges the stream. Poll has no failure cases here short of 
 * hanging on an infinitely reoccuring EAGAIN. 
 * note this is intended to be used with an input stream (ie, stdin, socket). 
 * Simple and concise, but can probably be done without importing poll */ 
#define spurge(fdno) \
	struct pollfd fds[1]; \
	fds[0].fd = (fdno); \
	while(poll(fds, 1, 0)) getchar();


#endif
