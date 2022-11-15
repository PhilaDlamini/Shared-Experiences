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
#include "header.h"

void sendInvalid(int sockfd) { //2, 4, 7, 8
    //Send the hello message: type, source, dest, length, id
    Header message = Header_new(5, "client4", "client4", 5, 4230);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
    char *more = "Some more random things that were not specified by the client in the eader";
    write(sockfd, more, strlen(more));
}

void sendHello(int sockfd) {
    //Send the hello message: type, source, dest, length, id
    Header message = Header_new(1, "client4", "Server", 0, 0);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
}

void sendClientReq(int sockfd) {
    //Send the hello message: type, source, dest, length, id
    Header message = Header_new(3, "client4", "Server", 0, 0);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
}

void recHelloAck(int sockfd) {
    //Read in the response from the server
    char response[50];
    bzero(response, 50);
    int n = read(sockfd, response, 50);
    Header ret_h = (Header) response;
    Header_toHostOrder(ret_h);
    if(ret_h->type == 2) 
        printf("HELLO_ACK received!\n");

    
    bzero(response, 50);
    n = read(sockfd, response, 50);
    ret_h = (Header) response;
    Header_toHostOrder(ret_h);
    if(ret_h->type == 4) 
        printf("CLIENT LIST received!\n");

    //Read in the client list
    char clients[400];
    bzero(clients, 400);
    n = read(sockfd, clients, 400);
    printf("Got these clients: ");

    for(int i = 0; i < n; i++) {
        printf("%c", clients[i]);
    }
    printf("\n");
}

void recClientList(int sockfd) {
    char response[50];
    bzero(response, 50);
    int n = read(sockfd, response, 50);
    Header ret_h = (Header) response;
    Header_toHostOrder(ret_h);
    if(ret_h->type == 4) 
        printf("CLIENT LIST received!\n");

    //Read in the client list
    char clients[400];
    bzero(clients, 400);
    n = read(sockfd, clients, 400);
    printf("Got these clients: ");

    for(int i = 0; i < n; i++) {
        printf("%c", clients[i]);
    }
    printf("\n");
}

void recSendError(int sockfd) {
    //Read in the response from the server
    char response[50];
    bzero(response, 50);
    int n = read(sockfd, response, 50);
    Header ret_h = (Header) response;
    Header_toHostOrder(ret_h);
    if(ret_h->type == 8) 
        printf("Mesage %d could not be delivered\n", ret_h->message_id);

}

void recClientPresent(int sockfd) {
    //Read in the response from the server
    char response[50];
    bzero(response, 50);
    int n = read(sockfd, response, 50);
    Header ret_h = (Header) response;
    Header_toHostOrder(ret_h);
    if(ret_h->type == 7) 
        printf("Client %s already present\n", ret_h -> dest);

}

void sendPartial(int sockfd, char *recipient) {
    char *msg1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis consectetur ligula ullamcorper lacus luctus tristique. ";
    char *msg2 = "Quisque faucibus, tortor eget malesuada pellentesque, urna diam dictum ligula, vitae interdum dui est ornare lectus. Cras ac turpis sed mi commodo venenatis a at sapien. Sed suscipit finibus lacus, quis fringilla ante ullamcorper vitae. Phasellus rhoncus sapien eros, vel. Yeah nice!";
    Header message = Header_new(5, "client4", recipient, 400, 1);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
    write(sockfd, msg1, strlen(msg1));
    write(sockfd, msg2, strlen(msg2));
    printf("Sent CHAT of len %d to %s\n", strlen(msg1) + strlen(msg2) , recipient);
}


void sendChat(int sockfd, char *recipient, char *msg, int ID) {
    int len = strlen(msg);
    Header message = Header_new(5, "client4", recipient, len, ID);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
    write(sockfd, msg, len);
   //if(recipient != NULL) printf("Sent CHAT of len %d to %s\n",len, recipient);
}

void sendExit(int sockfd) {
    //Send the hello message: type, source, dest, length, id
    Header message = Header_new(6, "client4", "Server", 0, 0);
    Header_toNetworkOrder(message);
    write(sockfd, message, sizeof(*message));
    printf("Send EXIT to server\n");
}

void recChat(int sockfd) {
    char res3[50];
    bzero(res3, 50);
    int n = read(sockfd, res3, 50);
    Header ret_h2 = (Header) res3;
    Header_toHostOrder(ret_h2);
    if(ret_h2->type == 5) 
        printf("CHAT received! Source: %s\n", ret_h2->source);

    //Read in the actual client list
    char m[ret_h2->length];
    bzero(m, ret_h2->length + 1);
    n = read(sockfd, m, ret_h2->length);
    printf("Got message: %s\n", m);
}

/*
 * Sends the request to the server
 */
char* start_client(int port) {
    struct sockaddr_in saddr;
    struct hostent *h;
    int sockfd, connfd;

    //Open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        printf("Error opening socket");
        exit(EXIT_FAILURE);
    }

    //Verify that the host name exists
    h = gethostbyname("localhost");
    if(h == NULL) {
        printf("Unknown host name\n");
        exit(EXIT_FAILURE);
    }

    //Build the server's internet address
    bzero((char *) &saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    memcpy((char *) &saddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    saddr.sin_port = htons(port);

    //Connect with server
    connfd = connect(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));
    if(connfd < 0) {
        printf("Error connecting with server\n");
        exit(EXIT_FAILURE);
    }

    sendHello(sockfd);
    recHelloAck(sockfd);
    
//    recClientPresent(sockfd);


    // recChat(sockfd);

    // sleep(30);

    char *msg1 = "Laughing my fucking ass off. What?!";
    sendChat(sockfd, "client2", msg1, 23);

     char *msg2 = "L?!";
    sendChat(sockfd, "client2", msg2, 23); 
    
    char *msg3 = "Ye. Wassup";
    sendChat(sockfd, "phila", msg3, 23);

    // char *msg2 = "Dang that's crazy bro";
    // sendChat(sockfd, "client2", msg2, 432);

    //sendPartial(sockfd, "client3");
      recChat(sockfd);
    recChat(sockfd);
    recChat(sockfd);
    sendExit(sockfd);
    close(connfd);
    close(sockfd);
}

int main(int argc, char *argv[]) {
     if(argc < 2) {
        printf("Server port required. Aborting...\n");
        exit(EXIT_FAILURE);
    }

    //Start the server
    start_client(atoi(argv[1]));
    return 0;
}