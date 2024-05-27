#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT_SE 12345
#define PORT_SCII 54321
#define MAX_CLIENTS 10
#define BUFFER_SIZE 500

typedef struct {
    int id;
    int prod_energy;
    int cons_energy;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;

void handle_client(int client_socket);
int add_client(int client_socket);
void remove_client(int client_socket);

int main() {
    int server_socket_se, server_socket_scii;
    struct sockaddr_in server_addr_se, server_addr_scii;
    fd_set active_fd_set, read_fd_set;
    int i, max_fd;

    // Inicializa o servidor SE
    server_socket_se = socket(AF_INET, SOCK_STREAM, 0);
    server_addr_se.sin_family = AF_INET;
    server_addr_se.sin_port = htons(PORT_SE);
    server_addr_se.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket_se, (struct sockaddr*)&server_addr_se, sizeof(server_addr_se));
    listen(server_socket_se, MAX_CLIENTS);

    // Inicializa o servidor SCII
    server_socket_scii = socket(AF_INET, SOCK_STREAM, 0);
    server_addr_scii.sin_family = AF_INET;
    server_addr_scii.sin_port = htons(PORT_SCII);
    server_addr_scii.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket_scii, (struct sockaddr*)&server_addr_scii, sizeof(server_addr_scii));
    listen(server_socket_scii, MAX_CLIENTS);

    // Configura os conjuntos de descritores
    FD_ZERO(&active_fd_set);
    FD_SET(server_socket_se, &active_fd_set);
    FD_SET(server_socket_scii, &active_fd_set);
    max_fd = server_socket_se > server_socket_scii ? server_socket_se : server_socket_scii;

    while (1) {
        read_fd_set = active_fd_set;
        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == server_socket_se || i == server_socket_scii) {
                    // Nova conexão
                    int client_socket = accept(i, NULL, NULL);
                    if (client_socket >= 0) {
                        FD_SET(client_socket, &active_fd_set);
                        if (client_socket > max_fd) {
                            max_fd = client_socket;
                        }
                        if (!add_client(client_socket)) {
                            close(client_socket);
                        }
                    }
                } else {
                    // Dados de um cliente existente
                    handle_client(i);
                    FD_CLR(i, &active_fd_set);
                    close(i);
                }
            }
        }
    }

    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("recv");
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Received: %s\n", buffer);

    // Processamento da mensagem aqui
    // ...
}

int add_client(int client_socket) {
    if (num_clients >= MAX_CLIENTS) {
        char error_msg[] = "ERROR: Client limit exceeded\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return 0;
    }

    clients[num_clients].id = client_socket;
    clients[num_clients].prod_energy = rand() % 31 + 20; // 20-50 mWh
    clients[num_clients].cons_energy = rand() % 101; // 0-100%
    num_clients++;

    char welcome_msg[] = "Welcome to the server\n";
    send(client_socket, welcome_msg, strlen(welcome_msg), 0);
    return 1;
}

void remove_client(int client_socket) {
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].id == client_socket) {
            clients[i] = clients[num_clients - 1];
            num_clients--;
            break;
        }
    }
}