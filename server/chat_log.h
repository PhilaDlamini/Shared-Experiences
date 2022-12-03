#include <stdlib.h>
#include <string.h>

#define INIT_LOG_CAPACITY 100

#define INIT_IMG_LOG_CAPACITY 100000
#include <stdlib.h>
#include <stdio.h>
/*
 * Stores the client IDs
 */
typedef struct ChatLog {
    long size;
    long capacity;
    char *chats;

    char *images;
    // short image_count;
    long images_size;
    long images_capacity;
} *ChatLog;

typedef struct __attribute__((__packed__)) Image {
    char type;
    char client_name [20];
    long size;
    char *contents;
} *Image;

ChatLog ChatLog_new();

void ChatLog_add(ChatLog log, char *to_add);
void ChatLog_add_image(ChatLog log, Image to_add);

void ChatLog_free(ChatLog log);