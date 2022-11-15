#include "header.h"

//Initializes a new header 
Header Header_new(short int type, char *source, char *dest, int length, int id) {
    Header header = malloc(sizeof(*header));
    header-> length = length;
    header-> message_id = id;
    header-> type = type;
    bzero(&(header->dest), 20);
    bzero(&(header->source), 20);
    if(source != NULL) strcpy(header-> source, source);
    if(dest != NULL) strcpy(header-> dest, dest);
    return header;
}

//Converts the header data into network order
void Header_toNetworkOrder(Header header) {
    header-> type = htons(header->type);
    header-> length = htonl(header->length);
    header-> message_id = htonl(header->message_id);
}

//Converts the header data into host order
void Header_toHostOrder(Header header) {
    header-> type = ntohs(header->type);
    header-> length = ntohl(header->length);
    header-> message_id = ntohl(header->message_id);
}

//Appends the message to the header and returns a it as a char *
char *append_message(Header header, char *data) {
    Header_toNetworkOrder(header);
    int data_len = strlen(data);
    char *message = malloc(sizeof(*header) + data_len + 1);
    strncpy(message, (char *) header, sizeof(*header));
    strcpy(message + sizeof(*header), data);
    message[sizeof(*header) + data_len] = '\0';
    return message;
}

//Deallocates the memory for the header 
void Header_free(Header header) {
    free(header);
}