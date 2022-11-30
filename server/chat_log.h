
#define INIT_LOG_CAPACITY 100

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