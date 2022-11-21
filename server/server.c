#include "server.h"
#include "signal.h"

int main (int argc, char *argv[]){
    if(argc < 2) {
        printf("Server port required. Aborting...\n");
        exit(EXIT_FAILURE);
    }

    printf("Starting server...\n");

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
        
        // printf("Waiting for a socket to be ready\n");

        /* Block until input arrives on an active socket */
        select(fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            
            /* Temp, immediately send video to client*/
            printf("Accepted client of fd %d. About to send movie\n", isock);

            //Add the socket to the master set
            FD_SET(isock, &master_set);
            fdmax = max(isock, fdmax);
        } else {
            for(int i = 0; i <= fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    bzero(buffer, MAX_MESSAGE_SIZE);
                    int n = read(i, buffer, MAX_MESSAGE_SIZE);

                    //Print out message 
                    if(n > 0) {
                        printf("server received %d bytes: %s\n", n, buffer);
                    }

                    process(buffer, i);

                    // printf("About to send video\n");
                    // send_video(isock);

                    // if (n > 0) 
                    //     //handle(container, clientIDs, buffer, n, i);
                    //     printf("todo: handle\n");
                    }
            }
        }
    }
    close(master_socket); 
}

void process(char *data, int fd) {

    char type = data[0];

    if(type == 1) {

        printf("Received HELLO message from %s\n", data + 1);

        //Send movies for now
        char movies[800];
        bzero(movies, 800);

        char *m = "Movie1\0Movie2\0Movie3\0Movie4\0";
        for(int i = 0; i < 28; i++){
            movies[i] = m[i];
        }

        char a[1];
        a[0] = 2;
        write(fd, a, 1);
        write(fd, movies, 800);

        printf("Sent movie list to %s\n", data + 1);
    } else if (type == 3) {
        printf("Got vote for movie index: %d\n", data[1]);
        printf("Tallying up votes...\n");

        // sleep(5);   //simulate vote tallying

        char b[2]; //Send the movie selected to the user 
        b[0] = 4;
        b[1] = 2; //movie of index 2 is selected
        write(fd, b, 2);

        //Send the movie to the client
        send_video(fd);

        //Send start signal 
        char a[1];
        a[0] = 7;
        long sec = 0;
        write(fd, a, 1);
        write(fd, &sec, 8); //seconds since started 

    } else if (type == 6) { //downloaded 

        printf("Got downloaded signal from client\n");
        //Send start signal 
        char a[1];
        a[0] = 7;
        write(fd, a, 1);

        long start = 0;
        start = htonll(start);
        write(fd, &start, sizeof(long));
        printf("Sent start duration to client\n");
    }
    
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

video_list get_videos() {
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
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("File size was %ld\n", file_size);

    //Send movie content message
    char a[1];
    a[0] = 5;
    write(port, a, 1);

    long original = file_size;
    file_size = htonll(file_size);
    printf("About to send movie size\n");
    write(port, &file_size, sizeof(long));
    
    // printf("file size %d", ntohl(file_size));
    printf("About to send movie\n");
    char *file_contents = malloc(original);
    int fd = fileno(f);
    printf("Created movie char *\n");
    read(fd, file_contents, original);
    printf("Read movie contents\n");
    write(port, file_contents , original);
    printf("Written movie contents\n");

    // fread(file_contents, 1, file_size, f);
    // send(port, file_contents, file_size, 0);

    //ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)


    for(int i = 0; i < 100; i++) 
        printf("%c", file_contents[i]);
    // printf("%s\n", file_contents);
    printf("Should have sent video\n");
}

int max(int a, int b) {
    return a > b ? a : b;
}