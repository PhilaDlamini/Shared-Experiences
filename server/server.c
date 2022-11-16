#include "server.h"
#include "signal.h"

int main (int argc, char *argv[]){
    if(argc < 2) {
        printf("Server port required. Aborting...\n");
        exit(EXIT_FAILURE);
    }

    //Start the server
    run_server(atoi(argv[1]));
    return 0;
}



void run_server(int port) {
    struct sockaddr_in caddr;
    int clen, isock = 0;
    int master_socket = make_socket(port);

    //Initialize the set of active fds
    int MAX_MESSAGE_SIZE = 1024;
    char buffer[MAX_MESSAGE_SIZE];
    fd_set master_set;
    fd_set temp_set;
    FD_ZERO(&master_set);
    FD_ZERO(&temp_set);

    FD_SET(master_socket, &master_set); 
    int fdmax = master_socket;

    signal(SIGPIPE, SIG_IGN);
    
    while(1) {
        
        //Copy master to temp 
        temp_set = master_set;


        /* Block until input arrives on an active socket */
        select(fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            
            /* Temp, immediately send video to client*/
            printf("About to send video\n");
            send_video(isock);

            //Add the socket to the master set
            FD_SET(isock, &master_set);
            fdmax = max(isock, fdmax);
        } else {
            for(int i = 0; i <= fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    bzero(buffer, MAX_MESSAGE_SIZE);
                    int n = read(i, buffer, MAX_MESSAGE_SIZE);
                    if (n > 0) 
                        //handle(container, clientIDs, buffer, n, i);
                        printf("todo: handle\n");
                    }
            }
        }
    }
    close(master_socket); 
}


int make_socket(int port) {
    struct sockaddr_in saddr;
    int master_socket = 0;
    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(master_socket < 0) {
        printf("Error creating port\n");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&optval , sizeof(int));

    memset(&saddr, '\0', sizeof(saddr)); //zero out the struct
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if(bind(master_socket, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        printf("Error binding\n");
        exit(EXIT_FAILURE);
    }

    if(listen(master_socket, 1) < 0) {
        printf("Error listening\n");
        exit(EXIT_FAILURE);
    }

    return master_socket;
}

video_list get_videos(){
    DIR *d;
    int list_capacity = 1;
    char **videos = malloc(list_capacity * sizeof(char *));
    int video_count = 0;

    struct dirent *dir;
    d = opendir(".");

    if (!d){
        printf("Error opening current directory\n");
        exit(EXIT_FAILURE);
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG &&
            strlen(dir->d_name) >= 4 && 
            strcmp(dir->d_name + (strlen(dir->d_name) - 4), 
                    ACCEPTED_VIDEO_FORMAT) == 0){
            if (video_count == (list_capacity - 1)){
                list_capacity *= 2;
                videos = realloc(videos, list_capacity * sizeof(char *));
            }
            char *file_name = malloc(strlen(dir->d_name) + 1);
            strcpy(file_name, dir->d_name);
            videos[video_count] = file_name;
            video_count++;
        }
    }
    closedir(d);
    videos = realloc(videos, video_count * sizeof(char *));
    video_list vds = {.videos = videos, .video_count = video_count};
    return vds;
}

void send_video(int port) {
    const char READ_ONLY_MODE [] = "r";
    video_list vds = get_videos();
    char *video_name = vds.videos[0];
    FILE *f = fopen(video_name, READ_ONLY_MODE);

    printf("Opened file %s \n", video_name);

    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("File size was %d\n", file_size);

    char *file_contents = malloc(file_size);
    fread(file_contents, 1, file_size, f);
    send(port, file_contents, file_size, 0);

    for(int i = 0; i < 1000; i++) 
        printf("%c", file_contents[i]);
    // printf("%s\n", file_contents);
    printf("Should have sent video\n");
}


int max(int a, int b) {
    return a > b ? a : b;
}