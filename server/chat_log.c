#include "chat_log.h"


ChatLog ChatLog_new(){
    ChatLog log = malloc(sizeof(*log));

    log-> size = 0;
    log-> capacity = INIT_LOG_CAPACITY;
    log->chats = malloc(INIT_LOG_CAPACITY);
    return log;
}

void ChatLog_add(ChatLog log, char *to_add){
    int to_add_size = strlen(to_add) + 1; /* +1 for '/0' */

    if (log->size + to_add_size >= log->capacity){
        log->chats = realloc(log->chats, (log->capacity * 2) + to_add_size);
    }

    strcpy(log->chats + log->size, to_add);
    log->size += to_add_size;
    log->chats[log->size] = '\0';
    return;
}