#include "server.h"

/*
 * Processes each message according to the message type
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
                        printf("Server received %d bytes\n", n);
                        process(buffer, i);
                    }
            }
        }

      }
    }

    close(master_socket); 

}

void process(char *data, int fd) {

    int type = data[0];

    if(type == 1) {

        printf("Got HELLO from %s\n", data);

        //send movie list 
        char list[801];
        bzero(list, 801);
        list[0] = 2;

        char *m = "MovieA\0MovieB\0MovieC\0MovieD\0";
        for(int i = 0; i < 28; i++)
            list[i + 1] = m[i];

        write(fd, list, 801);
        printf("Sent movie list to %s\n", data);

    } else if (type == 3) { //movie vote

        printf("Got vote for movie index %d\n", data[1]);
        printf("Tallying up votes\n");

        //Send the winning movie
        char s[2];
        s[0] = 4;
        s[1] = 2; //movie of index 2 selected
        write(fd, s, 2);
        printf("Sent movie voted for\n");

        //Send the actual video 
        char z[1];
        z[0] = 5;
        write(fd, z, 1); //Movie content type

        long size;
        char* contents = get_video("5sec.mp4", &size);
        size = htonll(size);
        printf("Movie size was %ld\n", htonll(size));
        write(fd, &size, sizeof(long));
        write(fd, contents, htonll(size));
        printf("sent movie to client\n");

    } else if (type == 6)  {//downloaded signal
        //Send start signal
        char c[1];
        c[0] = 7;
        write(fd, c, 1);

        long start = 0;
        start = htonll(start);
        write(fd, &start, sizeof(long));
        
    } else {
        printf("Server received unsupported message type\n");
    }
}

char* get_video(char *video_name, long *video_size) {
    const char READ_ONLY_MODE [] = "r";
    FILE *f = fopen(video_name, READ_ONLY_MODE);

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *video_contents = malloc(file_size);
    fread(video_contents, 1, file_size, f);
    fclose(f);
    *video_size = file_size;
    return video_contents;
}

int max(int a, int b) {
    return a > b ? a : b;
}
