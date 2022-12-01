#include <stdlib.h>
#include <string.h>

#define INIT_LOG_CAPACITY 100
#include <stdlib.h>
#include <stdio.h>
/*
 * Stores the client IDs
 */
typedef struct ChatLog {
    long size;
    long capacity;
    char *chats;
} *ChatLog;

ChatLog ChatLog_new();

void ChatLog_add(ChatLog log, char *to_add);

void free_log(ChatLog log);