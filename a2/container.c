#include "container.h"

/* Initializes the container
 */
Container Container_new() {
    Container container = malloc(sizeof(*container));
    container-> capacity = 0;
    container-> size = 0;
    container-> arr = NULL;
    return container;
}

/*
 * Clears the memory for the container 
 */
void Container_free(Container container) {
    for(int i = 0; i < container-> size; i++) 
        MessageBuffer_free(container->arr[i]);
    
    free(container->arr);
    free(container);
}

/* Inserts a partial message into the container 
 * Returns the size of the data that was inserted
 */
int Container_insert_partial(Container container, int clientFd, char *message, 
    int message_size) {

    //Get the client's message buffer
    for(int i = 0; i < container-> size; i++) {
        MessageBuffer curr = container->arr[i];
        if(curr->clientFd == clientFd) {
            return MessageBuffer_append(curr, message, message_size);
        }
    }
    
}

/*
 * Inserts a new message in tot he buffer
 * Returns the size of data inserted
 */
int Contianer_insert_new(Container container, int clientFd, char *message, int length, int full_length) {
    
    if(container-> size == container-> capacity) Container_expand(container);
    
    MessageBuffer buffer = MessageBuffer_new(clientFd, full_length);
    int sz = MessageBuffer_append(buffer, message, length);
    container->arr[container->size++] = buffer;
    return sz;
}


/*
 * Returns the MessageBuffer with the given file descriptor
 */
MessageBuffer getMessageBuffer(Container container, int clientFd) {

    for(int i = 0; i < container-> size; i++) {
        MessageBuffer curr = container->arr[i];
        if(curr-> clientFd == clientFd)
            return curr;
        
    }

    return NULL;
}

/*
 * Remove the buffer for the client 
 */
void Container_removeBuffer(Container container, int clientFd) {

    int i;
    int cleared = 0; 
    for(i = 0; i < container-> size; i++) {
        MessageBuffer curr = container->arr[i];
        if(clientFd == curr->clientFd) {
            MessageBuffer_free(container->arr[i]);
            cleared = 1;
            break;
        }
    }

    //Shift elements to the left
    int j;
    for(j = i; j < container->size; j++) 
        container->arr[j] = container->arr[j + 1];

    if(cleared) container->size--;
}


/*
 * Expands the container 
 */
void Container_expand(Container container) {
    container-> capacity = 2 * container->capacity + 2;
    MessageBuffer *arr = malloc(container-> capacity * sizeof(*arr));

    for(int i = 0; i < container->size; i++) 
        arr[i] = container-> arr[i];
    
    free(container->arr); 
    container->arr = arr;
}

//prints out 
void Container_print(Container container) {
    printf("Container size: %d\n", container->size);
    printf("Container capacity: %d\n", container->capacity);

    for(int i = 0; i < container-> size; i++) {
        MessageBuffer buf = container->arr[i];
        printf("%d: ", buf->clientFd);
        MessageBuffer_print(buf);
    }
}


/*
 * Discards all partials messages that are stale
 * Also takes in the record of clientIDs so we can remove clients from here
 */
void Container_discardStale(Container container, List clientIDs) {

    /* 
     * Find the list of stale entries and unopen fds
     */
    List list = List_new();

    for(int i = 0; i < container-> size; i++) {
        MessageBuffer curr = container->arr[i];
        int fd = curr->clientFd;

        if(isStale(curr) == 1) {
            char *id = List_getClientID(clientIDs, fd);
            List_add(list, id, fd);
            printf("Found stale partial message %s\n", id);
        }
    }

    //Remove them 
    for(int i = 0; i < list->size; i++) {
        Entry entry = list->data[i];
        int fd = entry->fd;

        Container_removeBuffer(container, fd);
        List_remove(clientIDs, entry->clientID);
        close(fd);
        printf("Removed %s from client record and closed connection\n", entry->clientID);
    }

    List_free(list);
}
