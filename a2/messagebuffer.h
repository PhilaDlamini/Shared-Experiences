#ifndef _MESSAGE_BUFFER_
#define _MESSAGE_BUFFER_
#define T MessageBuffer
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "header.h"

/* 
 * A buffer for the message
 */
typedef struct T {
    int full_size;
    int current_size;
    int clientFd; 
    unsigned long arrival; //The time the message came
    char *message;
} *T;

//Creates the buffer
extern T MessageBuffer_new(int clientFd, int full_size);

//Deallocates the message 
extern void MessageBuffer_free(T buffer);

//Returns true if the whole message has been received
extern int isComplete(T buffer);

//Appends the new data to the message
extern int MessageBuffer_append(T buffer, char *data, int size); 

//Prints out the message in the buffer
extern void MessageBuffer_print(T buffer);

//Returns the message header and sets data to point to the data
extern Header getHeader(MessageBuffer buffer, char **data);

//Returns true if the partial message is stale
extern int isStale(MessageBuffer buffer); 

//Returns the min of the two values 
extern int min(int a, int b);

//Helper function: returns the current time in nanoseconds
extern unsigned long ns();
#undef T
#endif