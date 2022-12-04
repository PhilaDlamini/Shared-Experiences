#include "server.h"
#include "list.h"



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
                        Image curr_image = NULL;
                        if (message_type != IMAGE){
                            read_entire_message(&buffer, i, message_type, &master_set);
                        } else {
                            curr_image = read_image(i);
                            fprintf(stderr, "Returned from read_image\n");
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
                            fprintf(stderr, "Before send_image\n");
                            send_image(curr_image, log, clientIDs, i, master_set, fdmax);
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

void send_chat(char *message_data, ChatLog log, List clientIDs, int port_no, fd_set *master_set, int *fdmax){
    long name_len = strlen(message_data + 1);
    long r_chat_len = strlen(message_data + 21);

    //Build the message struct 
    struct Message chats;
    chats.type = CHATS;
    long chat_len = htonll(r_chat_len + name_len + 1);
    memcpy(chats.data, &chat_len, sizeof(chat_len));
    memcpy(chats.data + sizeof(chat_len), message_data + 1, name_len);
    chats.data[sizeof(chat_len) + name_len] = ':';
    memcpy(chats.data + sizeof(chat_len) + name_len + 1, message_data + 21, r_chat_len);
    chats.data[sizeof(chat_len) + name_len + r_chat_len + 1] = '\0';
   
    // bytes_read = read(port_no, *data + 1, 28);
    // long bytes_to_read = 0;
    // memcpy(&bytes_read, *data + 21, 8);
    // bytes_to_read = htonll(bytes_to_read);
    // bytes_read = read(port_no, *data + 29, bytes_to_read);



    // printf("Message to send\n");
    // for(int i = 0; i <= 8 + name_len + r_chat_len + 1; i++) {
    //     if(chats.data[i] == '\0') printf("0");
    //     else printf("%c ", chats.data[i]);
    // }

    ChatLog_add(log, chats.data + sizeof(chat_len));
    send_to_all(*master_set, *fdmax, chats, 11 + name_len + r_chat_len);
}

void send_image(Image to_send, ChatLog log, List clientIDs, int port_no, fd_set master_set, int fdmax){
    fprintf(stderr, "In send image\n");
    fd_set write_set = master_set;
    select(fdmax + 1, NULL, &write_set, NULL, NULL);
    for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, &write_set)){
            int n = write(i, (char *) to_send, to_send->size + 29);
        }
    }
    fprintf(stderr, "Sent image, about to enter chatlog\n");
    ChatLog_add_image(log, to_send);
}

void handle_media_controls(char message_type, char *message_data, bool paused, 
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
        memcpy(&seconds, message_data + 1, sizeof(seconds));
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
                           Message movie_list_message, int video_index, char *message_data, 
                           char *video_contents, long video_size, 
                           struct timespec video_start_time, ChatLog log){

    struct Message read_message; 
    memcpy(&read_message, message_data, 21);
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

                long time_since_video_start =  curr_time.tv_sec - video_start_time.tv_sec;
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

void read_entire_message(char **data, int port_no, char message_type, fd_set *master_set){    
    fprintf(stderr, "In read entire message\n");
    int bytes_read = 0;
    switch(message_type) {
        case HELLO:;
            fprintf(stderr, "Reading entire Hello\n");
            bytes_read = read(port_no, *data + 1, 20);
            break;
        case VOTE:;
            fprintf(stderr, "Reading entire vote\n");
            bytes_read = read(port_no, *data + 1, 1);
            break;
        case DOWNLOADED:; 
            break;
        case END_MOVIE:;
            break;
        case SEEK:;
            bytes_read = read(port_no, *data + 1, 8);
            break;
        case CHAT:; //being handled?
            bytes_read = read(port_no, *data + 1, 420);
            break;
        case GOODBYE:;
            bytes_read = read(port_no, *data + 1, 20);
            break;
    }
}

Image read_image(int port_no){
    fprintf(stderr, "In read Image\n");
    Image new_image = malloc(sizeof(struct Image));
    new_image->type = IMAGE;

    char a[28];
    
    // int bytes_read = read(port_no, new_image + 1, 28);
    int bytes_read = read(port_no, a, 28);

    fprintf(stderr, "Bytes read: %d\n", bytes_read);

    fprintf(stderr, "Name is: %s\n", a);


    long bytes_to_read;
    memcpy(new_image->client_name, a, 20);
    memcpy(&bytes_to_read, a + 20, sizeof(long));
    printf("size %ld\n", htonll(bytes_to_read));
    new_image->size = htonll(bytes_to_read);
    

    printf("new image name: %s\n", new_image->client_name);
    printf("new image size: %ld\n", new_image->size);

    char *image_contents = malloc(new_image->size);
    bytes_read = read(port_no, image_contents, bytes_to_read);
    new_image->contents = image_contents;
    fprintf(stderr, "Image size: %ld\n", new_image->size);
    return new_image;
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