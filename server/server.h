#include <dirent.h> 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "list.h"

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

struct video_list {
    char **videos;
    unsigned int video_count;
};
typedef struct video_list video_list;

struct __attribute__((__packed__)) Message {
    char type;
    char data [800];
};
typedef struct Message Message;

#define ACCEPTED_VIDEO_FORMAT ".mp4"
//#define VOTING_PHASE_MAX_MESSAGE_LENGTH 21
#define MAX_MESSAGE_LENGTH 801
#define SECONDS_TO_VOTE 60

#define HELLO 1
#define MOVIES 2
#define VOTE 3
#define MOVIE_SELECTED 4
#define MOVIE_CONTENT 5
#define DOWNLOADED 6
#define START 7
#define END_MOVIE 8
#define GOODBYE 9
#define ERROR 10

// enum message_type {HELLO = 1, MOVIES, VOTE, MOVIE_SELECTED, 
// MOVIE_CONTENT, DOWNLOADED, START, END_MOVIE, GOODBYE, ERROR};
// typedef enum message_type message_type;

void run_server(int port);
int make_socket(int port);

int voting(video_list vds, List clientIDs, int master_socket, fd_set *master_set, int *fdmax);
void handle_voting(List clientIDs, List voted, Message movie_list, fd_set *master_set, int video_string_length, 
                   char *data, int* vote_tally, int curr_port);

void play_video(video_list vds,  List clientIDs, int to_play_index, 
                int master_socket, fd_set *master_set, int *fdmax);
void handle_download(List clientIDs, fd_set *master_set,  char *data, int curr_port, int *downloaded_count, 
                     Message movie_content_message, 
                     char *video_contents, int video_size);
void handle_playing(List clientIDs, fd_set *master_set, char *data, int curr_port, int *end_count, 
                     Message movie_content_message, char *video_contents, 
                     int video_size, struct timespec video_start_time);

void send_to_all(fd_set master_set, int fdmax, Message to_send, int message_size);
video_list get_videos();
char *load_video (char *video_name, long *video_size);
int max(int a, int b);