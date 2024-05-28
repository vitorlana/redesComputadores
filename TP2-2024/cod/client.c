#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#define BUFFER_SIZE 500

void communicate_with_servers(int se_socket, int scii_socket);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <se_port> <scii_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int se_socket, scii_socket;
    struct addrinfo hints, *res, *p;
    int status;
    int family;

    // Determinar a família do endereço com base no formato do IP
    if (strchr(argv[1], ':')) {
        family = AF_INET6; // IPv6
    } else {
        family = AF_INET; // IPv4
    }

    // Configuração do socket SE
    memset(&hints, 0, sizeof hints);
    hints.ai_family = family; // Definir a família de endereços
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Conecta ao servidor SE
    for (p = res; p != NULL; p = p->ai_next) {
        if ((se_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        if (connect(se_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(se_socket);
            perror("connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Falha na conexão ao servidor SE\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    // Configuração do socket SCII
    hints.ai_family = family; // Definir a família de endereços novamente
    if ((status = getaddrinfo(argv[1], argv[3], &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Conecta ao servidor SCII
    for (p = res; p != NULL; p = p->ai_next) {
        if ((scii_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        if (connect(scii_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(scii_socket);
            perror("connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Falha na conexão ao servidor SCII\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    communicate_with_servers(se_socket, scii_socket);

    close(se_socket);
    close(scii_socket);
    return 0;
}

void communicate_with_servers(int se_socket, int scii_socket) {
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
    int max_fd = se_socket > scii_socket ? se_socket : scii_socket;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(se_socket, &read_fds);
        FD_SET(scii_socket, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Leitura de entrada do usuário
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }

            // Verifica se a mensagem é "kill"
            if (strncmp(buffer, "kill", 4) == 0) {
                printf("Encerrando conexões...\n");
                send(se_socket, buffer, strlen(buffer), 0);
                send(scii_socket, buffer, strlen(buffer), 0);
                break;
            }

            // Envia a mensagem para ambos os servidores
            send(se_socket, buffer, strlen(buffer), 0);
            send(scii_socket, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(se_socket, &read_fds)) {
            // Recebe a mensagem do servidor SE
            int bytes_received = recv(se_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("From SE: %s\n", buffer);
            }
        }

        if (FD_ISSET(scii_socket, &read_fds)) {
            // Recebe a mensagem do servidor SCII
            int bytes_received = recv(scii_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("From SCII: %s\n", buffer);
            }
        }
    }
}
