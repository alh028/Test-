#include "http-server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

Chat chats[MAX_CHATS];
int chat_count = 0;

// Format chat entry with appropriate timestamp and reaction handling
void format_chat_entry(char *buffer, size_t size, Chat *chat) {
    char timestamp[20];
    struct tm *timeinfo = localtime(&chat->timestamp);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Format the message with id, timestamp, username, and the message
    snprintf(buffer, size, "[#%d %s]    %-15s: %s\n", 
             chat->id, timestamp, chat->username, chat->message);

    // Add reactions to the chat
    for (int i = 0; i < chat->reaction_count; i++) {
        char reaction_entry[64];
        snprintf(reaction_entry, sizeof(reaction_entry), 
                 "                              (%s)  %s\n",
                 chat->reactions[i].username, chat->reactions[i].reaction);
        strncat(buffer, reaction_entry, size - strlen(buffer) - 1);
    }
}

// Send the HTTP response back to the client
void send_http_response(int client_socket, const char *content) {
    char response[8192];
    int content_length = strlen(content);
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n%s",
             content_length, content);

    write(client_socket, response, strlen(response));
}

// Handle the /chats endpoint - output all chats
void handle_chats(int client_socket) {
    char response_body[8192] = "";
    for (int i = 0; i < chat_count; i++) {
        char chat_entry[512];
        format_chat_entry(chat_entry, sizeof(chat_entry), &chats[i]);
        strncat(response_body, chat_entry, sizeof(response_body) - strlen(response_body) - 1);
    }
    send_http_response(client_socket, response_body);
}

// Handle the /post endpoint - create a new chat message
void handle_post(int client_socket, const char *username, const char *message) {
    if (chat_count >= MAX_CHATS) {
        send_http_response(client_socket, "Error: Chat limit reached\n");
        return;
    }
    if (strlen(username) > MAX_USERNAME_LEN || strlen(message) > MAX_MESSAGE_LEN) {
        send_http_response(client_socket, "Error: Invalid parameters\n");
        return;
    }

    // Create a new chat entry
    Chat *new_chat = &chats[chat_count++];
    new_chat->id = chat_count;
    new_chat->timestamp = time(NULL);
    strncpy(new_chat->username, username, MAX_USERNAME_LEN);
    strncpy(new_chat->message, message, MAX_MESSAGE_LEN);
    new_chat->reaction_count = 0;

    // Output the chat entry to the terminal (for server-side logging)
    char chat_entry[512];
    format_chat_entry(chat_entry, sizeof(chat_entry), new_chat);
    printf("%s", chat_entry);

    // Return the updated chat list to the client
    handle_chats(client_socket);
}

// Handle the /react endpoint - add a reaction to a message
void handle_react(int client_socket, const char *username, const char *reaction, int chat_id) {
    if (chat_id < 1 || chat_id > chat_count || strlen(username) > MAX_USERNAME_LEN || strlen(reaction) > MAX_USERNAME_LEN) {
        send_http_response(client_socket, "Error: Invalid react parameters\n");
        return;
    }

    // Find the chat message and add the reaction
    Chat *chat = &chats[chat_id - 1];
    if (chat->reaction_count >= MAX_REACTIONS) {
        send_http_response(client_socket, "Error: Reaction limit reached\n");
        return;
    }

    Reaction *new_reaction = &chat->reactions[chat->reaction_count++];
    strncpy(new_reaction->username, username, MAX_USERNAME_LEN);
    strncpy(new_reaction->reaction, reaction, MAX_USERNAME_LEN);

    // Output the updated chat entry with reactions to the terminal
    char chat_entry[512];
    format_chat_entry(chat_entry, sizeof(chat_entry), chat);
    printf("%s", chat_entry);

    // Return the updated chat list to the client
    handle_chats(client_socket);
}

// Handle the /reset endpoint - reset all chats
void handle_reset(int client_socket) {
    chat_count = 0;
    send_http_response(client_socket, "Server reset successfully\n");
}
