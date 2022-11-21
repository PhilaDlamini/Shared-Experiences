#include "list.h"


//Creates a new instance of the list 
List List_new() {
    List list = malloc(sizeof(*list));
    list-> size = 0;
    list-> capacity = 0;
    list-> data = NULL;

    return list;
}

//Creates a new entry
Entry Entry_new(char *clientID, int fd) {
    Entry entry = malloc(sizeof(*entry));
    entry->clientID = clientID;
    entry->fd = fd;
    return entry;
}

//Deallocates memory for the list
void List_free(List list) {
    for(int i = 0; i < list-> size; i++) 
        Entry_free(list->data[i]);

    free(list->data);
    free(list);
}




//For adding into the list
void List_add(List list, char *clientID, int fd) {
    if(list-> capacity == list-> size) List_expand(list);
    Entry entry = Entry_new(clientID, fd);
    list->data[list->size++] = entry;
    
}

//For removing an ID from the list 
void List_remove(List list, char *clientId) {

    if(list->size == 0) return;

    //Find the index of the id 
    int index = 0;
    while(index < list-> size && strcmp(list->data[index]->clientID, clientId) != 0) index++;

    //Remove it 
    int removed = 0;
    if(index < list->size) {
        Entry_free(list->data[index]);
        removed = 1;
    }

    while(index < list->size) {
        list->data[index] = list->data[index + 1];
        index++;
    }

    if(removed) list->size--;
}

//Returns true if the string is already in the list 
int List_contains_id(List list, char *clientId) {
    for(int i = 0; i < list->size; i++) {
        if(strcmp(list->data[i]->clientID, clientId) == 0) return 1;
    }
    
    return 0;
}

//Returns true if the list fd is already in the list 
int List_contains_fd(List list, int fd) {
    for(int i = 0; i < list->size; i++) {
        if(list->data[i]->fd == fd) return 1;
    }
    
    return 0;
}

//Expands the list 
void List_expand(List list) {
    list-> capacity = 2 * list-> capacity + 2;
    Entry *data = malloc(sizeof(*data) * list-> capacity);

    for(int i = 0; i < list-> size; i++) 
        data[i] = list-> data[i];
    
    free(list->data);
    list->data = data;
    
}

//Helper funtion 
void List_print(List list) {
    int i;
    for(i = 0; i < list->size; i++) {
        Entry entry = list->data[i];
        printf("clientID:%s, fd: %d\n", entry->clientID, entry->fd);
    }
}

//Returns a continous character array
char *List_getIDs(List list, int *size) {

    int MAX_DATA = 400; //Messages are at most 400 bytes
    char *ids = malloc(MAX_DATA);
    bzero(ids, MAX_DATA);

    int offset = 0;
    for(int i = 0; i < list-> size; i++) {
        strcpy(ids + offset, list->data[i]->clientID);
        offset += strlen(list->data[i]->clientID) + 1;
    }

    *size = offset;
    return ids;
}


//Returns the fd associted with the clientID
int List_getFD(List list, char *clientID) {

    for(int i = 0; i < list->size; i++) {
        Entry entry = list->data[i];
        if(strcmp(entry->clientID, clientID) == 0) {
            return entry->fd;
        }
    }

    return -1;
}

//Returns the clientID associted with the fd
char* List_getClientID(List list, int fd) {

    for(int i = 0; i < list->size; i++) {
        Entry entry = list->data[i];
        if(entry->fd == fd) 
            return entry->clientID;
    }

    return NULL;
}


//Frees up the entry
void Entry_free(Entry entry) {
    if(entry == NULL) return;
    //free(entry->clientID); causes issues when the clientID was not malloced
    free(entry);
}