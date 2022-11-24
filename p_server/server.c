#include "server.h"

/*
 * A server that sends videos to clients over JavaFX's HTTP live streaming 
 */
int main(int argc, char *argv[]) {

    if(argc < 2) {
        printf("Server port required. Aborting...\n");
        exit(EXIT_FAILURE);
    }

    //Start the server
    start_server(atoi(argv[1]));
    return 0;
}

/*
 * If the message is complete, process it and then clear from the buffer
 * That is, call the destructor for it :) 
 */

//Makes the socket
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

//Starts the server
void start_server(int port) {
    struct sockaddr_in caddr;
    int clen, isock = 0;
    int master_socket = make_socket(port);

    //Initialize the set of active fds
    char buffer[MAX_MESSAGE_SIZE];
    fd_set master_set;
    fd_set temp_set;
    FD_ZERO(&master_set);
    FD_ZERO(&temp_set);

    FD_SET(master_socket, &master_set); 
    int fdmax = master_socket;

    signal(SIGPIPE, SIG_IGN);

    printf("Started server\n");
    
    while(1) {
        
        //Copy master to temp 
        temp_set = master_set;

        /* Block until input arrives on an active socket */
        select(fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) {
                printf("Error accepting client request\n");
                printf("%s\n", strerror(errno));
                exit(1);
            }

            printf("Accepted client of fd %d\n", isock);
            
            //Add the socket to the master set
            FD_SET(isock, &master_set);
            fdmax = max(isock, fdmax);
        } else {
            for(int i = 0; i <= fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    bzero(buffer, MAX_MESSAGE_SIZE);
                    int n = read(i, buffer, MAX_MESSAGE_SIZE);
                    if (n > 0)  {
                        printf("Server received %d bytes: %s\n", n, buffer);
                        send_video(i);
                    }
            }
        }

      }
    }

    close(master_socket); 

}

void send_video(int fd) {

    //Send the minecraft video
    FILE *f = fopen("minecraft.mp4", "r");

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *video_contents = malloc(file_size);
    fread(video_contents, 1, file_size, f);
    fclose(f);

    printf("video size was %ld\n", file_size);

    char *res = "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nContent-Length: 26478358\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n";
    write(fd, res, strlen(res));
   
    //Write the whole movie
    write(fd, video_contents, file_size);
}

int max(int a, int b) {
    return a > b ? a : b;
}