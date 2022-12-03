#include "server.h"

int init = 1;
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
                        process(buffer, i, n);
                    }
            }
        }

      }
    }

    close(master_socket); 

}

void process(char *data, int fd, int n) {

    int type = data[0];

    if(type == 1) {

        printf("Got HELLO from %s\n", data);

        // //send movie content
        // char z[1];
        // z[0] = 5;
        // write(fd, z, 1); //Movie content type

        //send movie list 
        char list[801];
        bzero(list, 801);
        list[0] = 2;

        char *m = "minecraft.mp4\0";
        for(int i = 0; i < 14; i++)
            list[i + 1] = m[i];

        write(fd, list, 801);
        printf("Sent movie list to %s\n", data);

        //First send image
        char z[1];
        z[0] = 16;
        write(fd, z, 1);

        char name[20];
        bzero(name, 20);
        char *n = "Phila";
        memcpy(name, n, 5);
        write(fd, name, 20);
        send_image(fd);

        //Send chats
        char b[1];
        b[0] = 15;
        write(fd, b, 1);
        long len = 70;
        len = htonll(len);
        write(fd, &len, sizeof(long));

        char *data = "Phila:hi everyone\0Sean:hey, how a u\0Nick:good! I love this movie:)\0";
        char f[70];
        memcpy(f, data, 67);
        short s = 0; //The short can be mistaken for '\0' when splitting! Bad design :(
        s = htons(s);
        memcpy(f + 67, &s, 2);
        f[69] = '\0';
        write(fd, f, htonll(len));

        //Then also send 
        printf("Sent all chats to them as well\n");

    } else if (type == 3) { //movie vote

        printf("Got vote for movie index %d\n", data[1]);
        printf("Tallying up votes\n");

        //Send the winning movie
        char s[2];
        s[0] = 4;
        s[1] = 0; //movie of index 0 selected
        write(fd, s, 2);
        printf("Sent movie voted for\n");

        //Send the actual video 
        char z[1];
        z[0] = 5;
        write(fd, z, 1); //Movie content type
        send_video(fd);

    } else if (type == 6)  {//downloaded signal

        printf("Got download signal from client\n");
        //Send start signal
        char c[1];
        c[0] = 7;
        write(fd, c, 1);

        long start = 0; //Start at 0 seconds
        start = htonll(start);
        write(fd, &start, sizeof(long));
        
    } else if (type == 8) {

        printf("Got END movie from %d\n", fd);

        //send movie list 
        char list[801];
        bzero(list, 801);
        list[0] = 2;

        char *m = "minecraft.mp4\0";
        for(int i = 0; i < 14; i++)
            list[i + 1] = m[i];

        write(fd, list, 801);
        printf("Sent movie list to %d\n", fd);

    } else if(type == 9) {

        printf("Got goodbye from %s\n", data);

    } else if(type == 10) { //toggle received. send to toggle to all clients

        printf("Got toggle from %s\n", data);
        char a[1];
        a[0] = 11;
        write(fd, a, 1);
    } else if(type == 12) {

        //TODO: why do we not need to do htonll here? 
        //Get long duration
        long duration; 
        memcpy(&duration, data + 1, sizeof (long)); 

        printf("Got seek to %ld from %s\n", duration, data + 9);
        
        //send seek to all clients
        char b[1];
        b[0] = 13;
        write(fd, b, 1);
        duration = htonll(duration);
        write(fd, &duration, sizeof(long));
    } else if(type == 14) {
        printf("Got chat from %s\n", data + 1);
        printf("Message was: %s\n", data + 21);
        
        //Send the chat to *all* clients
        char b[1];
        b[0] = 15;
        write(fd, b, 1);

        char *message = "Sean: Hi all! Loving this movie? This is my fav so far\0";
        long len = strlen(message) + 1;
        len = htonll(len);
        write(fd, &len, sizeof(long));
        write(fd, message, htonll(len));
    } else if (type == 16) {

        printf("Got image from %s\n", data + 1);
        printf("writing back %d bytes\n", n);

        //Write back the entire image
        write(fd, data, n);

    }
    else {
        printf("Server received unsupported message type\n");
    }
}

void send_image(int fd) {

    //Send the minecraft video
    FILE *f = fopen("img.jpg", "r");

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *video_contents = malloc(file_size);
    fread(video_contents, 1, file_size, f);
    fclose(f);

    printf("image size was %ld\n", file_size);

    long rev = htonll(file_size);
    write(fd, &rev, sizeof(long));
   
    //Write the whole movie
    write(fd, video_contents, file_size);
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

    long rev = htonll(file_size);
    write(fd, &rev, sizeof(long));
   
    //Write the whole movie
    write(fd, video_contents, file_size);
}

int max(int a, int b) {
    return a > b ? a : b;
}