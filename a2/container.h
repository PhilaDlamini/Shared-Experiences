#ifndef _CONTAINER_
#define _CONTAINER_
#include "messagebuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "list.h"

/*
 * A contianer for all partial messages 
 * Funtions like a simple HashTable: key-> clientID, value -> message
 */
typedef struct Container {
    int capacity;
    int size;
    MessageBuffer *arr;
} *Container;

//Initializes the container
extern Container Container_new();

/*
 * Clears the memory for the container 
 */
extern void Container_free(Container container);

/* 
 * Inserts a partial message into the container 
 */
extern int Container_insert_partial(Container contianer, int clientFd, char *message, int length);

/*
 * Inserts a new message in tot he buffer
 */
extern int Contianer_insert_new(Container container, int clientFd, char *message, int length, int full_length);

/*
 * Returns the MessageBuffer of the corresponding client
 * Or null is it doesn't exist
 */
extern MessageBuffer getMessageBuffer(Container container, int clientFd);

/*
 * Clears the buffer at the given index
 */
extern void Container_removeBuffer(Container container, int clientFd);

/*
 * Expands the container 
 */
extern void Container_expand(Container container);

/*
 * Discards all partials messages that are stale
 */
void Container_discardStale(Container container, List clientIDs);

//prints out 
extern void Container_print(Container container);

#endif