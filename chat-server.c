#include "http-server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

// Parse the incoming HTTP request and route it to the correct handler
void parse_request(const char *request, int client_socket) {
    if (strncmp(request, "GET /chats", 10) == 0) {
        handle_chats(client_socket);
    } else if (strncmp(request, "GET /post?", 10) == 0) {
        char username[16], message[256];
        sscanf(request, "GET /post?user=%15[^&]&message=%255s", username, message);
        handle_post(client_socket, username, message);
    } else if (strncmp(request, "GET /react?", 11) == 0) {
        char username[16], reaction[16];
        int chat_id;
        sscanf(request, "GET /react?user=%15[^&]&message=%15[^&]&id=%d", username, reaction, &chat_id);
        handle_react(client_socket, username, reaction, chat_id);
    } else if (strncmp(request, "GET /reset", 10) == 0) {
        handle_reset(client_socket);
    } else {
        send_http_response(client_socket, "Error: Invalid request\n");
    }
}

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : 8080;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);
    printf("Server started on port %d\n", port);

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        char request[1024];
        read(client_socket, request, sizeof(request) - 1);
        parse_request(request, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
