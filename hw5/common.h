/* Useful functions for homework by Nikita Georgiou  */ 

#ifndef NIKITA_COMMON_H
#define NIKITA_COMMON_H

#include <stdio.h>
#include <string.h>

/* Reads input from stdin into buf of up to size bytes. Input longer
 * than size is truncated to size - 1 (excluding null terminator).
 * Returns the number of bytes read, or -1 on error. Also prints error msg
 * on error. Purges stdin */ 
int psgets(char*, size_t);

/* currently unimplimented. fpurge is really useful for clearing backed-up
 * out of bounds input for functions like psgets, but isnt portable.
 * need to find a way to do this */ 
void spurge(FILE *stream);

#endif
