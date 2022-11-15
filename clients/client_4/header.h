#ifndef _MESSAGE_
#define _MESSAGE_
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h> 

/*
 * Defines the message structure
 */
typedef struct __attribute__((__packed__)) Header {

    //The header fields
    short int type;
    char source[20]; //20 byte long 
    char dest[20]; //20 byte long
    int length;
    int message_id;

} *Header;

//Initializes a new message type, source, dest, length, id, data
extern Header Header_new(short int type, char *source, char *dest, int length, int id);

//Deallocates the memory for the message 
extern void Header_free(Header message);

//Appends the message to the header and returns a it as a char *
extern char *append_message(Header header, char *data);

//Converts the message data into 
extern void Header_toNetworkOrder(Header message);
extern void Header_toHostOrder(Header message);


#endif