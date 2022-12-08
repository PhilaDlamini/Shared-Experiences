#include "server.h"
#include "list.h"

int skip_counter = 0;


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
    int clen = 0, isock = 0;
    int master_socket = make_socket(port);

    //Initialize the set of active fds
    int MAX_MESSAGE_SIZE = 1024;
    // char buffer[MAX_MESSAGE_SIZE];
    char *buffer = malloc(MAX_MESSAGE_SIZE);

    fd_set master_set;
    fd_set read_set;
    fd_set write_set;
    FD_ZERO(&master_set);
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);

    FD_SET(master_socket, &master_set); 
    int fdmax = master_socket;

    //Key states for media control chats
    int play_status = PLAYING;

    // signal(SIGPIPE, SIG_IGN);


    List clientIDs = List_new();
    ChatLog log = ChatLog_new();
    
    while (1){
        
        if (clientIDs->size == 0){
            ChatLog_free(log);
            log = ChatLog_new();
        }
        
        fprintf(stderr, "At top while loop\n");
        video_list vds = get_videos();
        int curr_phase = VOTING_PHASE;
 
        // bool playing_active = false;

        int downloaded_count = 0, ended_count = 0;

        /* Will be set to point to array containing contents of an mp4 file */
        char *video_contents = NULL;
        long video_size = 0; /* Will be set to length of mp4 file in bytes */
        /* Will be set to index of movie in vds that wins vote*/
        int video_index = 0; 

        bool paused = false;

        /* Will eventually be used to track time since start of video*/
        struct timespec playing_start_time;

        /***********************************************/
        /**************** Voting Set Up ****************/
        /***********************************************/

            Message movie_list_message;
            movie_list_message.type = MOVIES;
            bzero(movie_list_message.data, 800);
            
            /* TODO: Error if over 800 chars */
            
            /* 
            * Creates a string representation of the list of all available 
            * videos that can be sent to clients
            */
            int video_string_length = 0;
            int i = 0;
            for (i = 0; i < vds.video_count; i++){
                strcpy(movie_list_message.data + video_string_length, vds.videos[i]);
                video_string_length += strlen(vds.videos[i]) + 1; /* + 1 for \0 */
            }
            

            fd_set write_set; // TODO: Remove?
            write_set = master_set;
            if (clientIDs->size > 0){
                send_to_all(master_set, fdmax, movie_list_message, 
                            801);
            }

            /* Tracks clients who've voted and prevents one client making over 1 vote */
            List voted = List_new();
            int *vote_tally = malloc(vds.video_count * sizeof(int));
            bzero(vote_tally, vds.video_count * sizeof(int));

            struct timespec voting_start_time;
            timespec_get(&voting_start_time, TIME_UTC);

        /***********************************************/
        /************** End Voting Set Up **************/
        /***********************************************/


        while (curr_phase != END_CURRENT_VIDEO){
            fd_set temp_set = master_set;
            
            /* Timeout so server will not block forever at select */
            struct timeval *select_timeout = malloc(sizeof(struct timeval));
            select_timeout->tv_sec = 5;
            select_timeout->tv_usec = 0; 
            
            /* Block until input arrives on an active socket */
            select(fdmax + 1, &temp_set, NULL, NULL, select_timeout);

            /* Service all sockets with input pending */
            if(FD_ISSET(master_socket, &temp_set)) {
                printf("connected\n");
                isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
                if(isock < 0) 
                    printf("Error accepting client request\n");
                FD_SET(isock, &master_set);
                fdmax = max(isock, fdmax);
            } else {
                for(int i = 0; i <= fdmax; i++) {
                    if(FD_ISSET(i, &temp_set)) {
                        bzero(buffer, MAX_MESSAGE_LENGTH);
                        printf("reading from socket\n");
                        
                        char message_type = -1;
                        fprintf(stderr, "bf read\n");
                        int n = read(i, &message_type, 1);
                        fprintf(stderr, "message_type: %c\n", message_type + '0');
                        buffer[0] = message_type;

                        if (n <= 0){
                            fprintf(stderr, "In disconenct client\n");
                            close(i);
                            FD_CLR(i, &master_set);
                            // TODO: disconnect client
                        }
                        

                        /* Read in the rest of the message */
                        char *curr_image = NULL;
                        int size;
                        if (message_type != IMAGE){
                            read_entire_message(&buffer, i, message_type, &master_set);
                        } else {
                            handle_image(i, log);
                            // curr_image = read_image(i, &size);
                            // fprintf(stderr, "Returned from read_image\n");
                        }
                        
                        /* TODO: put in function? */

                        if (message_type == GOODBYE){
                            
                                
                            char *controlling_client = List_getClientID(clientIDs, i);
                            printf("Controlling client was %s\n", controlling_client);
                            int client_name_len = strlen(controlling_client);
                                
                            struct Message chats;
                            chats.type = CHATS;
                            char *message;
                            long chat_len;

                            message = ":LEFT"; //TODO: figure out if forward/backward
                            chat_len = htonll(client_name_len + strlen(message) + 1);
                            memcpy(chats.data, &chat_len, sizeof(chat_len));
                            memcpy(chats.data + sizeof(chat_len), controlling_client, client_name_len);
                            memcpy(chats.data + sizeof(chat_len) + client_name_len, message, strlen(message));
                            chats.data[sizeof(chat_len) + client_name_len + strlen(message)] = '\0';
                            
                            send_to_all(master_set, fdmax, chats, 10 + client_name_len + strlen(message));
                            ChatLog_add(log, chats.data + sizeof(chat_len));
                            
                            
                            fprintf(stderr, "In goodbyue\n");
                            List_remove(clientIDs, List_getClientID(clientIDs, i));
                            close(i);
                            FD_CLR(i, &master_set);
                        } else if (message_type == DOWNLOADED){
                            fprintf(stderr, "IN DOWNLOADED\n");
                            downloaded_count++;
                        } else if (message_type == END_MOVIE){
                            ended_count++;
                        } else if (curr_phase == VOTING_PHASE && message_type == VOTE){
                            int video_index = (int) buffer[1];
                            fprintf(stderr, "Got vote for index %d\n", video_index);
                            vote_tally[video_index]++;
                            List_add(voted, NULL, i);
                        } else if (message_type == HELLO) {
                            handle_client_joining(curr_phase, i, clientIDs, 
                                                  movie_list_message, video_index, buffer, 
                                                  video_contents, video_size, 
                                                  playing_start_time, log);
                        } else if (curr_phase == PLAYING_PHASE && (message_type == TOGGLE || message_type == SEEK)){
                            handle_media_controls(message_type, buffer, paused, clientIDs, i, &master_set, &fdmax, playing_start_time, log, &play_status);
                        } else if (message_type == CHAT){
                            send_chat(buffer, log, clientIDs, i, &master_set, &fdmax);
                        } else if (message_type == IMAGE){
                            continue;
                        }
                    }
                }
            }
             
            

            /* 
             * Tests whether current phase (either voting, download, or playing)
             * should end. If so, sets up next phase. 
             */
            if (curr_phase == VOTING_PHASE){
                fprintf(stderr, "Evaluating voting phase\n");
                struct timespec curr_time;
                timespec_get(&curr_time, TIME_UTC);
                if ((curr_time.tv_sec - voting_start_time.tv_sec) > 
                     SECONDS_TO_VOTE || (voted->size == clientIDs->size 
                     && voted->size >= 1))
                {
                    fprintf(stderr, "Voting Phase to Download Phase \n");
                    curr_phase = DOWNLOAD_PHASE;
                    
                    video_index = tally_votes(vote_tally, vds);
                    printf("video index %d\n", video_index);
                    fprintf(stderr, "Before sending movie to all\n");
                    send_movie_to_all(&master_set, &fdmax, clientIDs, vds, video_index, &video_contents, &video_size);
                    fprintf(stderr, "After sending movie to all\n");

                    List_free(voted);
                    free(vote_tally);
                }
            } else if (curr_phase == DOWNLOAD_PHASE && downloaded_count >= clientIDs->size){
                curr_phase = PLAYING_PHASE;

                timespec_get(&playing_start_time, TIME_UTC);
                Message start_message; 
                start_message.type = START;

                //long time_since_video_start =  htonll(playing_start_time.tv_sec);
                long time_since_video_start = 0;
                //memcpy(start_message.data, &time_since_video_start, sizeof(long));
                memcpy(start_message.data, &time_since_video_start, sizeof(long));
                printf("About to send start to all\n");
                send_to_all(master_set, fdmax, start_message, 1 + sizeof(long));
                fprintf(stderr, "finished sending start to all\n");
            } else if ((curr_phase == PLAYING_PHASE && ended_count >= clientIDs->size) 
                       || clientIDs->size == 0){
                curr_phase = END_CURRENT_VIDEO;
            }
            fprintf(stderr, "End loop iteration\n");
        }
        free (video_contents);
    }

    close(master_socket); 
}

void send_chat(char *message_header, ChatLog log, List clientIDs, int port_no, fd_set *master_set, int *fdmax){
    long name_len = strlen(message_header + 1);
    long r_chat_len = strlen(message_header + 21);

    //Build the message struct 
    struct Message chats;
    chats.type = CHATS;
    long chat_len = htonll(r_chat_len + name_len + 1);
    memcpy(chats.data, &chat_len, sizeof(chat_len));
    memcpy(chats.data + sizeof(chat_len), message_header + 1, name_len);
    chats.data[sizeof(chat_len) + name_len] = ':';
    memcpy(chats.data + sizeof(chat_len) + name_len + 1, message_header + 21, r_chat_len);
    chats.data[sizeof(chat_len) + name_len + r_chat_len + 1] = '\0';
   
    // bytes_read = read(port_no, *header + 1, 28);
    // long bytes_to_read = 0;
    // memcpy(&bytes_read, *header + 21, 8);
    // bytes_to_read = htonll(bytes_to_read);
    // bytes_read = read(port_no, *header + 29, bytes_to_read);



    // printf("Message to send\n");
    // for(int i = 0; i <= 8 + name_len + r_chat_len + 1; i++) {
    //     if(chats.header[i] == '\0') printf("0");
    //     else printf("%c ", chats.header[i]);
    // }

    ChatLog_add(log, chats.data + sizeof(chat_len));
    send_to_all(*master_set, *fdmax, chats, 11 + name_len + r_chat_len);
}

// void send_image(char* to_send, int size, ChatLog log, List clientIDs, int port_no, fd_set master_set, int fdmax){
//     fprintf(stderr, "In send image\n");
//     fd_set write_set = master_set;
//     // ChatLog_add_image(log, to_send);

//     printf("First 100 bytes we'll send\n");
//     for(int i = 0; i < 100; i ++)
//         printf("%c", to_send[i]);
//     printf("\n");
//     printf("Server will send %d bytes\n", size);

//     select(fdmax + 1, NULL, &write_set, NULL, NULL);
//     for (int i = 0; i <= fdmax; i++) {
//         if (FD_ISSET(i, &write_set)){
//             int n = write(i, to_send, size);

//             //manually send chats for now
//             long s = htonll(2);
//             char chats[2];
//             chats[0] = CHATS;
//             write(i, chats, 1);
//             write(i, &s, sizeof(long));
//             chats[0] = ':';
//             chats[1] = '\0';
//             write(i, chats, 2);
//         }
//     }
//     fprintf(stderr, "Sent image\n");
// }

void handle_media_controls(char message_type, char *message_header, bool paused, 
                      List clientIDs, int port_no, fd_set *master_set, 
                      int *fdmax, struct timespec video_start_time, ChatLog log, 
                      int *play_status){
    
    //struct Message read_message; 

    printf("In handle media controls\n");
    Message media_control_message;
    char *controlling_client = List_getClientID(clientIDs, port_no);
    printf("Controlling client was %s\n", controlling_client);
    int client_name_len = strlen(controlling_client);

    //For sending chats
    struct Message chats;
    chats.type = CHATS;
    char *message;
    long chat_len;

    if (message_type == TOGGLE){        
        media_control_message.type = TOGGLE_MOVIE;
        send_to_all(*master_set, *fdmax, media_control_message, 1);

        //Build message
        chats.type = CHATS;
        if(*play_status == PLAYING) {
            message = ":PAUSED MOVIE";
            *play_status = PAUSED;
        } else  {
            message = ":RESUMED MOVIE";
            *play_status = PLAYING;
        }

        chat_len = htonll(client_name_len + strlen(message) + 1);
        memcpy(chats.data, &chat_len, sizeof(chat_len));
        memcpy(chats.data + sizeof(chat_len), controlling_client, client_name_len);
        memcpy(chats.data + sizeof(chat_len) + client_name_len, message, strlen(message));
        chats.data[sizeof(chat_len) + client_name_len + strlen(message)] = '\0';

    } else if (message_type == SEEK){
        
        /* TODO: send backward or forward */

        long seconds;
        char direction;
        memcpy(&seconds, message_header + 1, sizeof(seconds));
        memcpy(&direction, message_header + 29, sizeof(direction));

        if (direction == BACKWARD){
            fprintf(stderr, "SEEKED BACKWARD\n");
            skip_counter++;
        } else if (direction == FORWARD) {
            fprintf(stderr, "SEEKED FOREWARD\n");
            skip_counter--;
        }

        media_control_message.type = SEEK_MOVIE;
        memcpy(media_control_message.data, &seconds, sizeof(seconds));
        send_to_all(*master_set, *fdmax, media_control_message, 9);
        
        //Build struct (perhaps this could be a function?)
        message = ":SEEKED MOVIE"; //TODO: figure out if forward/backward
        chat_len = htonll(client_name_len + strlen(message) + 1);

        memcpy(chats.data, &chat_len, sizeof(chat_len));
        memcpy(chats.data + sizeof(chat_len), controlling_client, client_name_len);
        memcpy(chats.data + sizeof(chat_len) + client_name_len, message, strlen(message));
        chats.data[sizeof(chat_len) + client_name_len + strlen(message)] = '\0';
        
    }

    //Nofity clients and log event
    send_to_all(*master_set, *fdmax, chats, 10 + client_name_len + strlen(message));
    ChatLog_add(log, chats.data + sizeof(chat_len));
}

void handle_client_joining(int curr_phase, int port_no, List clientIDs, 
                           Message movie_list_message, int video_index, char *message_header, 
                           char *video_contents, long video_size, 
                           struct timespec video_start_time, ChatLog log){

    struct Message read_message; 
    memcpy(&read_message, message_header, 21);
    char *new_client = malloc(20);
    strcpy(new_client, read_message.data);
    List_add(clientIDs, new_client, port_no);

    //fprintf(stderr, "In voting hello\n");
    int n = write(port_no, (char *) &movie_list_message, 801);

    if (curr_phase != VOTING_PHASE){
        Message movie_selected_message;
        bzero(&movie_selected_message, sizeof(movie_selected_message));
        movie_selected_message.type = MOVIE_SELECTED;
        movie_selected_message.data[0] = (char) video_index;
        write(port_no, (char *) &movie_selected_message, 2);
        
        Message movie_content_message; 
        movie_content_message.type = MOVIE_CONTENT;
        
        long video_size_to_send =  htonll(video_size);
        memcpy(movie_content_message.data, &video_size_to_send, sizeof(long));
        write(port_no, (char *) &movie_content_message, 1 + sizeof(long));
        fprintf(stderr, "Video size is %lu\n", video_size);
        write(port_no, video_contents, video_size);

        if (curr_phase == PLAYING_PHASE){
            char message_type;
            read(port_no, &message_type, 1);
            if (message_type == DOWNLOADED){
                Message start_message; 
                start_message.type = START;
                struct timespec curr_time;
                timespec_get(&curr_time, TIME_UTC);

                long time_since_video_start =  curr_time.tv_sec - video_start_time.tv_sec + (10 * skip_counter);
                fprintf(stderr, "time since video started: %ld\n", time_since_video_start);
                time_since_video_start = htonll(time_since_video_start);
                memcpy(start_message.data, &time_since_video_start, sizeof(long));
                write(port_no, (char *) &start_message, 1 + sizeof(long));
            }
        }
    }

    /* Send all prior chat messages */
    write(port_no, log->images, log->images_size); /* First send images */
    struct Message chat_log_message; 
    chat_log_message.type = CHATS;
    long chat_log_size_to_send = htonll(log->size);
    memcpy(chat_log_message.data, &chat_log_size_to_send, 
           sizeof(chat_log_size_to_send));
    memcpy(chat_log_message.data + sizeof(chat_log_size_to_send), log->chats, log->size);
    
    char *arr = (char *) &chat_log_message;
    for(int i = 0; i < (9 + log->size); i++) {
        printf("%c ", (arr[i] + '0'));
    }
    write(port_no, (char *) &chat_log_message, 9 + log->size);
}

void read_entire_message(char **header, int port_no, char message_type, fd_set *master_set){    
    fprintf(stderr, "In read entire message\n");
    int bytes_read = 0;
    switch(message_type) {
        case HELLO:;
            fprintf(stderr, "Reading entire Hello\n");
            bytes_read = read(port_no, *header + 1, 20);
            break;
        case VOTE:;
            fprintf(stderr, "Reading entire vote\n");
            bytes_read = read(port_no, *header + 1, 1);
            break;
        case DOWNLOADED:; 
            break;
        case END_MOVIE:;
            break;
        case SEEK:;
            bytes_read = read(port_no, *header + 1, 29);
            break;
        case CHAT:; //being handled?
            bytes_read = read(port_no, *header + 1, 420);
            break;
        case GOODBYE:;
            bytes_read = read(port_no, *header + 1, 20);
            break;
    }
}

void handle_image(int port_no, ChatLog log) {
    fprintf(stderr, "In read Image\n");    
    char long_text[20];
    char curr = '\0';
    long bytes_read = 0;
    while (curr != ':'){
        int byte_read = read(port_no, &curr, 1);
        fprintf(stderr, "In while loop: %c\n", curr);
        long_text[bytes_read] = curr;
        bytes_read += byte_read;
    }
    
    short long_string_length = bytes_read; /* Length of string representation of image size */
    long_text[bytes_read] = '\0';
    long img_size = strtol(long_text, NULL, 10);
    printf("Img size is %ld\n", img_size);

    //Read header
    int header_size = 1 + bytes_read + 20;
    char *header = malloc(header_size);
    bzero(header, header_size);
    header[0] = IMAGE;
    memcpy(header + 1, long_text, bytes_read);
    int n = read(port_no, header + 1 + bytes_read, 20);
    printf("Bytes read for username %d\n", n);

    //Read image 
    bytes_read = 0;
    char *bytes = malloc(img_size);
    while (bytes_read < img_size){
        bytes_read += read(port_no, bytes + bytes_read, img_size - bytes_read);
    }
    printf("Bytes read for img bytes %d\n", bytes_read);

    //write back 
    write(port_no, header, header_size);
    write(port_no, bytes, img_size);

    //Write the chat
    char chats[11];
    chats[0] = CHATS;
    long chat_size = htonll(2);
    char *str = ":\0";
    memcpy(chats + 1 , &chat_size, sizeof(long));
    memcpy(chats + 9, str, 2);
    write(port_no, chats, 11); 

    //Save to log
    char *to_save = malloc(header_size + img_size);
    memcpy(to_save, header, header_size);
    memcpy(to_save + header_size, bytes, img_size);
    
    ChatLog_add_image(log, to_save, header_size + img_size); 
    free(to_save);
    free(header);
}

void send_movie_to_all(fd_set *master_set, int *fdmax, List clientIDs, 
                       video_list vds, int video_index, char **video_contents, 
                       long *video_size){
    Message movie_selected_message;
    bzero(&movie_selected_message, sizeof(movie_selected_message));
    movie_selected_message.type = MOVIE_SELECTED;
    movie_selected_message.data[0] = (char) video_index;
    
    send_to_all(*master_set, *fdmax, movie_selected_message, 2);
    fprintf(stderr, "After sending movie_selected message\n");

    *video_contents = load_video(vds.videos[video_index], video_size);
    
    Message movie_content_message; 
    movie_content_message.type = MOVIE_CONTENT;
    long video_size_to_send =  htonll(*video_size);
    fprintf(stderr, "Video size: %lu   video_size_to_send: %lu\n", *video_size, video_size_to_send);
    memcpy(movie_content_message.data, &video_size_to_send, sizeof(long));
    
    fd_set write_set = *master_set;
    if (clientIDs->size > 0){
        fprintf(stderr, "In send-movie-to-all if\n");
        select(*fdmax + 1, NULL, &write_set, NULL, NULL);
        for(int i = 0; i <= *fdmax; i++) {
            if(FD_ISSET(i, &write_set)){
                int n = write(i, (char *) &movie_content_message, 1 + sizeof(long));
                fprintf(stderr, "n is %d\n", n);
                n = write(i, *video_contents, *video_size);
            }
        }
    }
}

int tally_votes(int *vote_tally, video_list vds){
    int max_votes = 0, max_index = 0;
    for (int i = 0; i < vds.video_count; i++){
        if (vote_tally[i] > max_votes){
            max_votes = vote_tally[i];
            max_index = i;
        }
    }
    return max_index;
}

video_list get_videos(){
    /* TODO: Error if over 255 movies */
    fprintf(stderr, "In get videos\n");
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
    fprintf(stderr, "Video count is %d\n", video_count);
    return vds;
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

void send_to_all(fd_set master_set, int fdmax, Message to_send, int message_size){
    fprintf(stderr, "in send_to_all\n");
    printf("Message size is %d\n", message_size);
    // for (int i = 0; i <= fdmax; i++) {

    //     if (FD_ISSET(i, &master_set)){
    //         fprintf(stderr, "Type is: %c\n", to_send.type + '0');
            
    //         fprintf(stderr, "actually sending\n");
    //         int n = write(i, &to_send, message_size);
    //         fprintf(stderr, "n is %d\n", n);
    //         exit(1);
    //     }
    // }
    
    fd_set write_set = master_set;
    select(fdmax + 1, NULL, &write_set, NULL, NULL);

    printf("Message size is %d\n", message_size);

    for(int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &write_set)){
            fprintf(stderr, "Type is: %c\n", to_send.type + '0');
            int n = write(i, (char *) &to_send, message_size);
        }
    }
}

char *load_video (char *video_name, long *video_size){
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