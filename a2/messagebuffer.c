#include "messagebuffer.h"

//Creates the buffer
MessageBuffer MessageBuffer_new(int clientFd, int full_size) {
    MessageBuffer buffer = malloc(sizeof(*buffer));
    buffer-> full_size = full_size;
    buffer-> current_size = 0;
    buffer-> clientFd = clientFd;
    buffer-> message = malloc(full_size);
    buffer-> arrival = ns();
    return buffer;
}

//Deallocates the message 
void MessageBuffer_free(MessageBuffer buffer) { 
    if(buffer == NULL) return;
    free(buffer-> message);
    //TODO: don't free client IDs cause they are not malloc'd?
    free(buffer);
}

//Returns true if the whole message has been received
int isComplete(MessageBuffer buffer) {
    if(buffer == NULL) return 0;
    return buffer->full_size == buffer-> current_size;
}

/* 
 * Appends the new data to the message
 * And returns the size of the data that was appended
 */
int MessageBuffer_append(MessageBuffer buffer, char *data, int size) {
    
    int times = min(size, (buffer->full_size - buffer-> current_size));
    for(int i = 0; i < times; i++) {
        buffer->message[buffer->current_size++] = data[i];
    }

    //Update the arrival time
    buffer-> arrival = ns();
    return times;
}

//Prints out the message in the buffer
void MessageBuffer_print(MessageBuffer buffer) {
    for(int i = 0; i < buffer->current_size; i++) {
        printf("%c", buffer->message[i]);
    }
    printf("\nMessage complete: %d\n", isComplete(buffer));
}

//Returns the header of this message buffer
Header getHeader(MessageBuffer buffer, char **data) {
    Header header = (Header) buffer-> message;
    *data = &(buffer->message[sizeof(*header)]);
    return header;
}

//Returns true if the partial message is stale
int isStale(MessageBuffer buffer) {
    return ns() > (buffer->arrival + 6e+10);
}

//Helper function: returns the current time in nanoseconds
unsigned long ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_nsec + (ts.tv_sec * 1e+9);
}

//Returns the min of the two values 
int min(int a, int b) {
    return a < b ? a : b;
}
