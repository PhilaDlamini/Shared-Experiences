#ifndef _SERVER_
#define _SERVER_
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include "list.h"
#include "messagebuffer.h"
#include "container.h"
#include "header.h"
#include <signal.h>
#include <errno.h>

/*
 * Enum for identifying message types
 */
enum message_type {HELLO = 1, HELLO_ACK, LIST_REQUEST, CLIENT_LIST, 
CHAT, EXIT, CLIENT_PRESENT, CANNOT_DELIVER};
typedef enum message_type message_type;

//Forward declare all methods (move these to a header file??)
int make_socket(int port);
void start_server(int port);
void process_message(message_type type);
void handle(Container container, List clientsIDs, char *data, int length, int fd);
void process(Container container, List clientsIDs, MessageBuffer buffer, int fd);
void hello(List clientIDs, Header header, int fd);
void client_list(List clientIDs, char *clientID, int fd);
void client_present(List clientIDs, char *clientID, int fd);
void chat(Header header, List clientIDs, char *message, int fd, int length);
int authenticate(List clientIDs, char *clientID, int fd);
int validateHeader(Header header);
void validateWrite(List clientIDs, char *clientID, int n, int fd);
void removeClient(List clientIDs, char *clientID, int fd);
int max(int a, int b);
#endif