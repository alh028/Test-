#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <time.h>

#define MAX_CHATS 100000
#define MAX_REACTIONS 100
#define MAX_USERNAME_LEN 15
#define MAX_MESSAGE_LEN 255

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char reaction[MAX_USERNAME_LEN + 1];
} Reaction;

typedef struct {
    int id;
    time_t timestamp;
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
    Reaction reactions[MAX_REACTIONS];
    int reaction_count;
} Chat;

void handle_chats(int client_socket);
void handle_post(int client_socket, const char *username, const char *message);
void handle_react(int client_socket, const char *username, const char *reaction, int chat_id);
void handle_reset(int client_socket);

#endif // HTTP_SERVER_H
