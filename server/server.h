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
#include "chat_log.h"

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
#define MAX_MESSAGE_LENGTH 1024
#define SECONDS_TO_VOTE 60

#define VOTING_PHASE 1
#define DOWNLOAD_PHASE 2
#define PLAYING_PHASE 3
#define END_CURRENT_VIDEO 4

/* Message Types */
#define PAUSED 0
#define PLAYING 1
#define HELLO 1
#define MOVIES 2
#define VOTE 3
#define MOVIE_SELECTED 4
#define MOVIE_CONTENT 5
#define DOWNLOADED 6
#define START 7
#define END_MOVIE 8
#define GOODBYE 9
#define TOGGLE 10
#define TOGGLE_MOVIE 11
#define SEEK 12
#define SEEK_MOVIE 13
#define CHAT 14
#define CHATS 15
#define IMAGE 16


/* TODO: ERROR */


// enum message_type {HELLO = 1, MOVIES, VOTE, MOVIE_SELECTED, 
// MOVIE_CONTENT, DOWNLOADED, START, END_MOVIE, GOODBYE, ERROR};
// typedef enum message_type message_type;

void run_server(int port);
int make_socket(int port);

void handle_client_joining(int curr_phase, int port_no, List clientIDs, 
                           Message movie_list_message, int video_index, char *message_data, 
                           char *video_contents, long video_size, 
                           struct timespec video_start_time, ChatLog log);
void handle_media_controls(char message_type, char *message_data, bool paused, 
                           List clientIDs, int port_no, fd_set *master_set, 
                           int *fdmax, struct timespec video_start_time, ChatLog log, 
                           int *play_status);

void read_entire_message(char **data, int port_no, char message_type, fd_set *master_set);
Image read_image(int port_no);

void send_movie_to_all(fd_set *master_set, int *fdmax, List clientIDs, 
                       video_list vds, int video_index, char **video_contents, 
                       long *video_size);

int tally_votes(int *vote_tally, video_list vds);

video_list get_videos();
char *load_video (char *video_name, long *video_size);

void send_to_all(fd_set master_set, int fdmax, Message to_send, int message_size);

void send_chat(char *message_data, ChatLog log, List clientIDs, int port_no, fd_set *master_set, int *fdmax);
void send_image(Image to_send, ChatLog log, List clientIDs, int port_no, fd_set master_set, int fdmax);

int max(int a, int b);