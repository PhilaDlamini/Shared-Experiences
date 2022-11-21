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
    int clen, isock = 0;
    int master_socket = make_socket(port);

    //Initialize the set of active fds
    int MAX_MESSAGE_SIZE = 1024;
    char buffer[MAX_MESSAGE_SIZE];
    fd_set master_set;
    fd_set read_set;
    fd_set write_set;
    FD_ZERO(&master_set);
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);

    FD_SET(master_socket, &master_set); 
    int fdmax = master_socket;

    signal(SIGPIPE, SIG_IGN);

    List clientIDs = List_new();
    
    while (1){
        video_list vds = get_videos();
        fprintf(stderr, "PLEASE PRINT\n");
        fprintf(stderr, "Video list size is %d\n", vds.video_count);
        /* Index of the vds.movies array that has the vote-winning movie */
        int video_index = voting(vds, clientIDs, master_socket, &master_set, &fdmax);
        fprintf(stderr, "Done with voting\n");
        play_video(vds, clientIDs, video_index, master_socket, &master_set, &fdmax);
    }

    close(master_socket); 
}


void play_video(video_list vds,  List clientIDs, int to_play_index, 
                int master_socket, fd_set *master_set, int *fdmax){
    fprintf(stderr, "In play_video\n");
    Message movie_selected_message;
    movie_selected_message.type = MOVIE_SELECTED;
    movie_selected_message.data[0] = to_play_index;
    

    send_to_all(*master_set, *fdmax, movie_selected_message, 2);

    long video_size;
    char *video_contents = load_video(vds.videos[to_play_index], &video_size);
    
    
    Message movie_content_message; 
    movie_content_message.type = MOVIE_CONTENT;
    fprintf(stderr, "Video size is %lu\n", video_size);
    long video_size_to_send =  htonll(video_size);
    fprintf(stderr, "Video size to send is %lu\n", video_size_to_send);
    memcpy(movie_content_message.data, &video_size_to_send, sizeof(long));
    
    fd_set write_set = *master_set;
    if (clientIDs->size > 0){
        select(*fdmax + 1, NULL, &write_set, NULL, NULL);
        for(int i = 0; i <= *fdmax; i++) {
            if(FD_ISSET(i, &write_set)){
                int n = write(i, (char *) &movie_content_message, 1 + sizeof(long));
                n = write(i, video_contents, video_size);
            }
        }
    }

    struct sockaddr_in caddr;
    int downloaded_count, clen, isock = 0;
    char buffer [MAX_MESSAGE_LENGTH];

    while (downloaded_count < clientIDs->size){
        fd_set temp_set = *master_set;
        /* Block until input arrives on an active socket */
        select(*fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            FD_SET(isock, &master_set);
            *fdmax = max(isock, *fdmax);
        } else {
            for(int i = 0; i <= *fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    printf("reading from socket\n");
                    int n = read(i, buffer, sizeof(char));             
                    if (n > 0) {
                        handle_download(clientIDs, master_set, buffer, i, &downloaded_count, 
                                        movie_content_message, 
                                        video_contents, video_size);
                    }
                    bzero(buffer, MAX_MESSAGE_LENGTH);
                }
            }
        }
    }

    struct timespec video_start_time;
    timespec_get(&video_start_time, TIME_UTC);
    Message start_message; 
    start_message.type = START;
    long time_since_video_start =  htonll(video_start_time.tv_sec);
    
    memcpy(start_message.data, &time_since_video_start, sizeof(long));
    printf("About to send start to all\n");
    send_to_all(*master_set, *fdmax, start_message, 1 + sizeof(long));

    int end_count = 0;
    while (end_count < clientIDs->size){
        fd_set temp_set = *master_set;
        /* Block until input arrives on an active socket */
        select(*fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            FD_SET(isock, &master_set);
            *fdmax = max(isock, *fdmax);
        } else {
            for (int i = 0; i <= *fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    int n = read(i, buffer, MAX_MESSAGE_LENGTH);             
                    if (n > 0) {
                        int n = read(i, buffer, sizeof(char));             
                        if (n > 0) {
                            handle_playing(clientIDs, master_set, buffer, i, &downloaded_count, 
                                           movie_content_message, video_contents, 
                                           video_size, video_start_time);
                        }
                        bzero(buffer, MAX_MESSAGE_LENGTH);
                    }
                }
            }
        }
    }
    free(video_contents);
    return;
}


void send_to_all(fd_set master_set, int fdmax, Message to_send, int message_size){
    fd_set write_set = master_set;
    select(fdmax + 1, NULL, &write_set, NULL, NULL);
    for(int i = 0; i <= fdmax; i++) {
        if(FD_ISSET(i, &write_set)){
            fprintf(stderr, "Type is: %c\n", to_send.type + '0');
            int n = write(i, (char *) &to_send, message_size);
        }
    }
}


int voting(video_list vds, List clientIDs, int master_socket, fd_set *master_set, int *fdmax){
    Message movie_list_message;
    movie_list_message.type = MOVIES;
    
    /* TODO: Error if over 800 chars */
    
    /* 
     * Create a string representation of the list of all available videos that
     * can be sent to clients
     */
    int video_string_length = 0;
    int i = 0;
    for (i = 0; i < vds.video_count; i++){
        strcpy(movie_list_message.data + video_string_length, vds.videos[i]);
        video_string_length += strlen(vds.videos[i]) + 1; /* + 1 for \0 */
    }
    bool voting_active = true;
    

    fd_set write_set;
    write_set = *master_set;
    if (clientIDs->size > 0){
        send_to_all(*master_set, *fdmax, movie_list_message, 
                    1 + video_string_length);
    }

    /* Tracks clients who've voted and prevents one client making over 1 vote */
    List voted = List_new();
    int *vote_tally = malloc(vds.video_count);
    bzero(vote_tally, vds.video_count);
    
    char buffer[MAX_MESSAGE_LENGTH ];
    struct sockaddr_in caddr;
    int clen, isock = 0;

    struct timespec voting_start_time;
    timespec_get(&voting_start_time, TIME_UTC);

    while (voting_active){
        printf("In voting loop\n");
        fd_set temp_set = *master_set;
        /* Block until input arrives on an active socket */
        select(*fdmax + 1, &temp_set, NULL, NULL, NULL);

        /* Service all sockets with input pending */
        if(FD_ISSET(master_socket, &temp_set)) {
            printf("connected\n");
            isock = accept(master_socket, (struct sockaddr *) &caddr, &clen);
            if(isock < 0) 
                printf("Error accepting client request\n");
            FD_SET(isock, master_set);
            *fdmax = max(isock, *fdmax);
        } else {
            for(int i = 0; i <= *fdmax; i++) {
                if(FD_ISSET(i, &temp_set)) {
                    printf("reading from socket\n");
                    bzero(buffer, MAX_MESSAGE_LENGTH );
                    int n = read(i, buffer, MAX_MESSAGE_LENGTH );             
                    if (n > 0) {
                        handle_voting(clientIDs, voted, movie_list_message, master_set,
                                      video_string_length, buffer, vote_tally, i);
                    }
                }
            }
        }
        struct timespec curr_time;
        timespec_get(&curr_time, TIME_UTC);
        if ((curr_time.tv_sec - voting_start_time.tv_sec) > SECONDS_TO_VOTE ||
            (voted->size == clientIDs->size && voted->size >= 1))
        {
            fprintf(stderr, "In voting exit\n");
            voting_active = false;
        }
    }

    int max_votes, max_index = 0;
    for (int i = 0; i < vds.video_count; i++){
        if (vote_tally[i] > max_votes){
            max_votes = vote_tally[i];
            max_index = i;
        }
    }
    free(vote_tally);
    return max_index;
}


void handle_voting(List clientIDs, List voted, Message movie_list, fd_set *master_set,
                   int video_string_length, char *data, int* vote_tally, 
                   int curr_port){
    printf("In handle voting\n");
    struct Message read_message; 
    memcpy(&read_message, data, MAX_MESSAGE_LENGTH );
    if (read_message.type == HELLO){
        fprintf(stderr, "In hello\n");
        char *new_client = malloc(20);
        strcpy(new_client, read_message.data);
        List_add(clientIDs, new_client, curr_port);
        int n = write(curr_port, (char *) &movie_list, 1 + video_string_length);
    } else if (read_message.type == VOTE && !List_contains_fd(voted, curr_port)){
        fprintf(stderr, "Got vote\n");
        int video_index = (int)  read_message.data[0];
        vote_tally[video_index]++;
        List_add(voted, NULL, curr_port);
    } else if (read_message.type == GOODBYE){
        List_remove(clientIDs, List_getClientID(clientIDs, curr_port));
        close(curr_port);
        FD_CLR(curr_port, master_set);
    }
}

void handle_download(List clientIDs, fd_set *master_set,  char *data, int curr_port, int *downloaded_count, 
                     Message movie_content_message, 
                     char *video_contents, int video_size){
    struct Message read_message; 
    memcpy(&read_message, data, MAX_MESSAGE_LENGTH);
    if (read_message.type == DOWNLOADED){
        (*downloaded_count)++;
    } else if (read_message.type == HELLO){
        char *new_client = malloc(20);
        strcpy(new_client, read_message.data);
        List_add(clientIDs, new_client, curr_port);
        
        int n = write(curr_port, (char *) &movie_content_message, 1 + sizeof(long));
        n = write(curr_port, video_contents, video_size);
    } else if (read_message.type == GOODBYE){
        List_remove(clientIDs, List_getClientID(clientIDs, curr_port));
        close(curr_port);
        FD_CLR(curr_port, master_set);
    }
}

void handle_playing(List clientIDs, fd_set *master_set, char *data, int curr_port, int *end_count, 
                     Message movie_content_message, char *video_contents, 
                     int video_size, struct timespec video_start_time)
{
    struct Message read_message; 
    memcpy(&read_message, data, MAX_MESSAGE_LENGTH);

     if (read_message.type == END_MOVIE){
        (*end_count)++;
    } else if (read_message.type == HELLO){
        char *new_client = malloc(20);
        strcpy(new_client, read_message.data);
        List_add(clientIDs, new_client, curr_port);
        
        int n = write(curr_port, (char *) &movie_content_message, 1 + sizeof(long));
        n = write(curr_port, video_contents, video_size);

        Message start_message; 
        start_message.type = START;
        long time_since_video_start =  htonll(video_start_time.tv_sec);
        memcpy(start_message.data, &time_since_video_start, sizeof(long));
        n = write(curr_port, (char *) &start_message, 1 + sizeof(long));
    } else if (read_message.type == GOODBYE){
        List_remove(clientIDs, List_getClientID(clientIDs, curr_port));
        close(curr_port);
        FD_CLR(curr_port, master_set);
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

/* 
 * Populates a video_list struct with the names of all the videos in the 
 * server's directory
 */
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