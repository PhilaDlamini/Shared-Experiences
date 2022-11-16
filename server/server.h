#include <dirent.h> 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>


#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ACCEPTED_VIDEO_FORMAT ".mp4"

struct video_list {
    char **videos;
    unsigned int video_count;
};
typedef struct video_list video_list;

void run_server(int port);
int make_socket(int port);
video_list get_videos();
int max(int a, int b);