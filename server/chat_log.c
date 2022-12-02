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
        log->chats = realloc(log->chats, (log->capacity * 2) + to_add_size); //issue here?
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

void ChatLog_free(ChatLog log){
    free(log->chats);
    free(log);
}