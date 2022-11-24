#ifndef _LIST_H
#define _LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/*
 * An entry into the list
 */
typedef struct Entry {
    char *clientID; //client IDs serve as the primary ID still
    int fd; //The fd of the client
} *Entry;

/*
 * Stores the client IDs
 */
typedef struct List {
    int size;
    int capacity;
    //char **data; //Array of strings
    Entry *data; //Changed from list of strings to list of entries
} *List;

//Creates a new entry 
extern Entry Entry_new(char *clientID, int fd);

//Creates a new instance of the list 
extern List List_new();

//Deallocates memory for the list
extern void List_free(List list);

//For adding into the list
extern void List_add(List list, char *clientId, int fd);

//For removing an ID from the list 
extern void List_remove(List list, char *clientId);

//Expands the list 
extern void List_expand(List list);

//Helper funtion 
extern void List_print(List list);

//Returns true if an entry with the client ID is already in the list 
extern int List_contains(List list, char *clientId);

//Returns true if an entry with the fd is already in the list
extern int List_contains_fd(List list, int fd);

//Returns a continous character array of all the client IDs
extern char *List_getIDs(List list, int *size);

//Returns the fd associted with the clientID
extern int List_getFD(List list, char *clientID);

//NOTE: returns NULL if the fd entry is not in the list (may cause issues?)
extern char* List_getClientID(List list, int fd);

//Frees the entry 
extern void Entry_free(Entry entry);

#endif