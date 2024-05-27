#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 500

void communicate_with_server(int client_socket);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    communicate_with_server(client_socket);

    close(client_socket);
    return 0;
}

void communicate_with_server(int client_socket) {
    char buffer[BUFFER_SIZE];
    printf("Enter a message: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    send(client_socket, buffer, strlen(buffer), 0);

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("recv");
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Received: %s\n", buffer);
}