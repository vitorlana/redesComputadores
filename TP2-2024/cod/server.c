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

void handle_client(int client_socket, int server_type, fd_set *active_fd_set);

int main(int argc, char *argv[]) {
    int server_socket;
    struct sockaddr_in server_addr4;
    struct sockaddr_in6 server_addr6;
    fd_set active_fd_set, read_fd_set;
    int i, max_fd, port, server_type;
    int family;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <v4|v6> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "v4") == 0) {
        family = AF_INET;
    } else if (strcmp(argv[1], "v6") == 0) {
        family = AF_INET6;
    } else {
        fprintf(stderr, "Invalid address family. Use v4 for IPv4 or v6 for IPv6.\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]);
    if (port == PORT_SE) {
        server_type = 1; // SE
    } else if (port == PORT_SCII) {
        server_type = 2; // SCII
    } else {
        fprintf(stderr, "Invalid port. Use %d for SE or %d for SCII.\n", PORT_SE, PORT_SCII);
        exit(EXIT_FAILURE);
    }

    // Inicializa o servidor
    server_socket = socket(family, SOCK_STREAM, 0);
    if (family == AF_INET) {
        memset(&server_addr4, 0, sizeof(server_addr4));
        server_addr4.sin_family = family;
        server_addr4.sin_port = htons(port);
        server_addr4.sin_addr.s_addr = INADDR_ANY;
        bind(server_socket, (struct sockaddr*)&server_addr4, sizeof(server_addr4));
    } else {
        memset(&server_addr6, 0, sizeof(server_addr6));
        server_addr6.sin6_family = family;
        server_addr6.sin6_port = htons(port);
        server_addr6.sin6_addr = in6addr_any;
        bind(server_socket, (struct sockaddr*)&server_addr6, sizeof(server_addr6));
    }

    listen(server_socket, MAX_CLIENTS);

    // Configura os conjuntos de descritores
    FD_ZERO(&active_fd_set);
    FD_SET(server_socket, &active_fd_set);
    max_fd = server_socket;

    printf("Servidor %s iniciado na porta %d (%s)\n", server_type == 1 ? "SE" : "SCII", port, argv[1]);

    while (1) {
        read_fd_set = active_fd_set;
        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == server_socket) {
                    // Nova conexão
                    int client_socket = accept(server_socket, NULL, NULL);
                    if (client_socket >= 0) {
                        FD_SET(client_socket, &active_fd_set);
                        if (client_socket > max_fd) {
                            max_fd = client_socket;
                        }
                        printf("Novo cliente conectado: %d\n", client_socket);
                    }
                } else {
                    // Dados de um cliente existente
                    handle_client(i, server_type, &active_fd_set);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket, int server_type, fd_set *active_fd_set) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("recv");
        close(client_socket);
        FD_CLR(client_socket, active_fd_set);
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Recebido do cliente %d: %s\n", client_socket, buffer);

    // Lógica específica para cada tipo de servidor
    if (server_type == 1) {
        // Lógica do servidor SE
        if (strncmp(buffer, "REQ_INFOSE", 10) == 0) {
            // Simula a produção de energia
            int prod_energy = rand() % 31 + 20; // 20-50 mWh
            snprintf(buffer, BUFFER_SIZE, "RES_INFOSE %d", prod_energy);
        } else if (strncmp(buffer, "kill", 4) == 0) {
            // Encerrar conexão
            snprintf(buffer, BUFFER_SIZE, "Servidor SE encerrando conexão.");
            send(client_socket, buffer, strlen(buffer), 0);
            close(client_socket);
            FD_CLR(client_socket, active_fd_set);
            return;
        }
    } else if (server_type == 2) {
        // Lógica do servidor SCII
        if (strncmp(buffer, "REQ_INFOSCII", 12) == 0) {
            // Simula o consumo de energia
            int cons_energy = rand() % 101; // 0-100%
            snprintf(buffer, BUFFER_SIZE, "RES_INFOSCII %d", cons_energy);
        } else if (strncmp(buffer, "kill", 4) == 0) {
            // Encerrar conexão
            snprintf(buffer, BUFFER_SIZE, "Servidor SCII encerrando conexão.");
            send(client_socket, buffer, strlen(buffer), 0);
            close(client_socket);
            FD_CLR(client_socket, active_fd_set);
            return;
        }
    }

    // Envia a resposta de volta para o cliente
    send(client_socket, buffer, strlen(buffer), 0);
}
