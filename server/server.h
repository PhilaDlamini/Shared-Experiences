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
#include <signal.h>
#include <errno.h>
#define MAX_MESSAGE_SIZE 1024
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))


//Forward declare all methods (move these to a header file??)
int make_socket(int port);
void start_server(int port);
void process(char *data, int fd);
char* get_video(char *video_name, long *video_size);
int max(int a, int b);

#endif