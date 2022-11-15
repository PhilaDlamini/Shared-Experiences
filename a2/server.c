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

    //Contianer for all partial messages + list of client IDs
    Container container = Container_new();
    List clientIDs = List_new();

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

        /* Remove any stale partial messages */
        Container_discardStale(container, clientIDs);

        /* Block until input arrives on an active socket */
        select(fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            
            //Add the socket to the master set
            FD_SET(isock, &master_set);
            fdmax = max(isock, fdmax);
        } else {
            for(int i = 0; i <= fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    bzero(buffer, MAX_MESSAGE_SIZE);
                    int n = read(i, buffer, MAX_MESSAGE_SIZE);
                    if (n > 0) 
                        handle(container, clientIDs, buffer, n, i);
            }
        }

      }
    }

    close(master_socket); 

}

/*
 * First layer of processing for messages received from client
 * Inserts the data into the message buffer
 */
void handle(Container container, List clientIDs, char *data, int length, int fd) {

    //Check to see if a partial message already was inserted
    MessageBuffer buf = getMessageBuffer(container, fd);

    //If it exists, append
    int size = 0;
    if(buf != NULL) 
        size = Container_insert_partial(container, fd, data, length);
    else { //Else, insert a new one
        Header header = (Header) data;
        Header_toHostOrder(header);        
        int full_length = sizeof(*header) + header->length;
        Header_toNetworkOrder(header);
        size = Contianer_insert_new(container, fd, data, length, full_length);
    }

    //If the message is complete, process it
    buf = getMessageBuffer(container, fd);

    if(isComplete(buf)) {
        process(container, clientIDs, buf, fd);
    }

    //If there was more data to process, handle that too 
    if(length - size > 0) 
        handle(container, clientIDs, data + size, length - size, fd);
    
}

//Processes each message based on the type
void process(Container container, List clientIDs, MessageBuffer buffer, int fd) {

    //Get the message type
    char *data;
    Header header = getHeader(buffer, &data);
    Header_toHostOrder(header);

    int type = header-> type;
    char *clientID = header-> source;

    //Validate header
    if(validateHeader(header)) {

        //Call the appropriate method based on it
        switch(type) {

            case HELLO:
                printf("Got HELLO from %s\n", clientID);
                hello(clientIDs, header, fd);
            break;

            case LIST_REQUEST:
                printf("Got LIST REQUEST from %s\n", clientID);
                
                if(authenticate(clientIDs, clientID, fd))
                    client_list(clientIDs, clientID, fd);
            break;

            case CHAT:
                printf("Got CHAT from %s\n", clientID);
                
                if(authenticate(clientIDs, clientID, fd))
                    chat(header, clientIDs, data, fd, header->length);
            break;
            
            case EXIT:  
                printf("Got EXIT from %s\n", clientID);
                
                if(authenticate(clientIDs, clientID, fd))
                    List_remove(clientIDs, clientID);
            break;

            default:
                printf("Unknown case in switch\n");
            break;
        }

    } else {
        printf("Couldn't process malformed header. Client removed\n");
        removeClient(clientIDs, clientID, fd);
    }

    //Remove the buffer from the container 
    Container_removeBuffer(container, buffer-> clientFd);
}

//Methods corresponding to each message type
void hello(List clientIDs, Header header, int fd) {
    char *clientID = malloc(20);
    bzero(clientID, 20);
    strcpy(clientID, header->source);

    //Disallow NULL clientIDs
    if(strlen(clientID) == 0) {
        printf("Invalid clientID. Closed connection\n");
        removeClient(clientIDs, clientID, fd);
        return;
    }

    //If the clientID already exists, send back the error
    if(List_contains(clientIDs, clientID)) {
        printf("clientID %s already in use.\n", clientID);
        client_present(clientIDs, clientID, fd);
        return;
    }

    //Else, register the client and send a HELLO_ACK
    List_add(clientIDs, clientID, fd);
    Header head_ret = Header_new(HELLO_ACK, "Server", clientID, 0, 0);
    Header_toNetworkOrder(head_ret);

    int n = write(fd, head_ret, sizeof(*head_ret));
    validateWrite(clientIDs, clientID, n, fd);

    //Also send the list of participating client IDs
    client_list(clientIDs, clientID, fd);
}


void client_list(List list, char *clientID, int fd) {

    int size;
    char *ids = List_getIDs(list, &size);
    Header header = Header_new(CLIENT_LIST, "Server", clientID, size, 0);
    Header_toNetworkOrder(header);

    int n = write(fd, header, sizeof(*header));
    validateWrite(list, clientID, n, fd);
    
    n = write(fd, ids, size); 
    validateWrite(list, clientID, n, fd);

}

void chat(Header header, List clientIDs, char *message, int fd, int length) {
    
    char *recipient = header->dest;
    char *source = header->source;
    int message_id = header->message_id;
    int n;


      /*
     * If the destination is empty or same as client,
     * remove client from register and close conn
     */
    if(strcmp(source, recipient) == 0) {
        printf("Error in chat: recipient same as source\n");
        removeClient(clientIDs, source, fd);
        return;
    } 

    /*
     * Message will be delivered only if the recipient is known
     */
    if(List_contains(clientIDs, recipient)) {
        int msg_size = strlen(message);
        int recipientFd = List_getFD(clientIDs, recipient); 
        Header head_n = Header_new(CHAT, source, recipient, length, message_id);
        Header_toNetworkOrder(head_n);
        n = write(recipientFd, head_n, sizeof(*head_n));
        validateWrite(clientIDs, recipient, n, recipientFd);
        n = write(recipientFd, message, length);
        validateWrite(clientIDs, recipient, n, recipientFd);

    } else {

        //Send ERROR_CANNOT_DELIVER 
        printf("Recipient not known. Closing connection with sender\n");
        Header head_n = Header_new(CANNOT_DELIVER, "Server", source, 0, message_id);
        Header_toNetworkOrder(head_n);
        n = write(fd, head_n, sizeof(*head_n));
        validateWrite(clientIDs, source, n, fd);

        //Remove client
        removeClient(clientIDs, source, fd);
    }
}

/*
 * If the client already exists, close connection with client
 * Note: existing clientID is not removed
 */
void client_present(List clientIDs, char *clientID, int fd) {

    //Get the fd in our records that's associated with the fd
    int recordedFd = List_getFD(clientIDs, clientID);

    if(recordedFd == fd) {
        printf("Client sent HELLO twice\n");
        removeClient(clientIDs, clientID, fd);
    } else {
        Header header = Header_new(CLIENT_PRESENT, "Server", clientID, 0, 0);
        Header_toNetworkOrder(header);
        write(fd, header, sizeof(*header));
        printf("NEW client provided clientID in use\n");
        close(fd);
    }
}

/*
 * Authenticate the client
 */
int authenticate(List clientIDs, char *clientID, int fd) {
    int fdRecorded = List_getFD(clientIDs,clientID);

    if(List_contains(clientIDs, clientID) &&
        fdRecorded == fd) 
            return 1;

    printf("Client %s could not be authenticated. Closing connection\n", clientID);
    removeClient(clientIDs, clientID, fd);
    return 0;
}

/*
 * Returns true if the header is valid
 */
int validateHeader(Header header) {
    if(header == NULL) return 0;

    int type = header->type;
    int length = header->length;
    return (type == HELLO || type == LIST_REQUEST ||
        type == CHAT || type == EXIT) && (length >= 0 && length <= 450);
}

/*
 * Removes the client from our records and closes the connection
 */
void removeClient(List clientIDs, char *clientID, int fd) {
    List_remove(clientIDs, clientID);
    close(fd);
}

/*
 * Validates the write to the client
 * If we couldn't write, remove the client 
 */
void validateWrite(List clientIDs, char *clientID, int n, int fd) {
    if(n < 0) {
        removeClient(clientIDs, clientID, fd);
        printf("Client %s's pipe was broken. Removed client\n", clientID);
    }
}

int max(int a, int b) {
    return a > b ? a : b;
}