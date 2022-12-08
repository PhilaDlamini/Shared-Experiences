#include "chat_log.h"


ChatLog ChatLog_new(){
    ChatLog log = malloc(sizeof(*log));

    log->size = 0;
    log->capacity = INIT_LOG_CAPACITY;
    log->chats = malloc(INIT_LOG_CAPACITY);

    // log->image_count = 0;
    log->images_size = 0;
    log->images_capacity = INIT_IMG_LOG_CAPACITY;
    log->images = malloc(INIT_IMG_LOG_CAPACITY);

    return log;
}

void ChatLog_add(ChatLog log, char *to_add){
    int to_add_size = strlen(to_add) + 1; /* +1 for '/0' */

    if (log->size + to_add_size >= log->capacity){
        log->capacity = (log->capacity * 2) + to_add_size;
        log->chats = realloc(log->chats, log->capacity); //issue here?
    }

    strcpy(log->chats + log->size, to_add);
    log->size += to_add_size;
    log->chats[log->size] = '\0';

    // printf("Log after appending: \n");
    // for(int i = 0; i < log->size; i++) {
    //     if(log->chats[i] == '\0') printf("0");
    //     else printf("%c",log->chats[i]);
    // }
    // printf("\n");
    return;
}

// void ChatLog_add_image(ChatLog log, Image to_add){
//     long to_add_size = to_add->size + 29;
//     fprintf(stderr, "To add size: %ld\n", to_add_size);
//     fprintf(stderr, "ADD IMAGE 1\n");
//     /* Convert image size back to network order to prepare for sending */
//     //to_add->size = htonll(to_add_size);

//     if (log->images_size + to_add_size >= log->images_capacity){
//         log->capacity = (log->images_capacity * 2) + to_add_size;
//         log->images = realloc(log->images, log->images_capacity);
//     }
//     fprintf(stderr, "ADD IMAGE 2\n");
//     /* Copy image message excluding actual image contents */
//     memcpy(log->images + log->images_size, to_add, 29);
//     log->images_size += 29;
//     fprintf(stderr, "ADD IMAGE 3\n");
//     /* Copy actual image contents excluding header */
//     memcpy(log->images + log->images_size, to_add->contents, to_add_size - 29);
//     fprintf(stderr, "ADD IMAGE 3.5\n");
//     log->images_size += (to_add_size - 29);
    
//     //free(to_add->contents);
//     fprintf(stderr, "ADD IMAGE 4\n");
//     /* 
//      * Add a placeholder for the image in the main log so that the image's 
//      * place in the order of all received chats is recorded 
//      */
//     // log->image_count++;
//     //char image_count_data[2]
//     char *image_delimiter = ":";
//     // image_delimiter[0] = ':';
//     //char image_delimiter [1] = ":";
//     ChatLog_add(log, image_delimiter);
// };


void ChatLog_add_image(ChatLog log, char *image, long bytes_to_add){
    if (log->images_size + bytes_to_add >= log->images_capacity){
        log->capacity = (log->images_capacity * 2) + bytes_to_add;
        log->images = realloc(log->images, log->images_capacity);
    }
    memcpy(log->images + log->images_size, image, bytes_to_add);
    log->images_size += bytes_to_add;
    char *image_delimiter = ":";
    ChatLog_add(log, image_delimiter);
};

void ChatLog_free(ChatLog log){
    free(log->images);
    free(log->chats);
    free(log);
}