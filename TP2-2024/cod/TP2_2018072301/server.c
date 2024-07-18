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
#define HISTORY_SIZE 100

typedef struct {
    int socket;
    int id;
    int active;
} Client;

Client clients[MAX_CLIENTS];
int next_client_id = 1;

int energy_production_history[HISTORY_SIZE];
int consumption_history[HISTORY_SIZE];
int energy_production_index = 0;
int consumption_index = 0;

void handle_client(int client_socket, int server_type, fd_set *active_fd_set, int *num_clients);
void add_energy_production(int value);
void add_consumption(int value);
void generate_random_energy_production();
void generate_random_consumption();
void handle_req_up(int client_socket);
void handle_req_none(int client_socket);
void handle_req_down(int client_socket);

int main(int argc, char *argv[]) {
    int server_socket;
    struct sockaddr_in server_addr4;
    struct sockaddr_in6 server_addr6;
    fd_set active_fd_set, read_fd_set;
    int i, max_fd, port, server_type;
    int family, num_clients = 0;

    generate_random_energy_production(); // Adiciona um novo valor de produção de energia (simulado)
    generate_random_consumption(); // Adiciona um novo valor de consumo de energia (simulado)

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

    // Inicializa os clientes
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
        clients[i].id = -1;
        clients[i].active = 0;
    }

    // Configura os conjuntos de descritores
    FD_ZERO(&active_fd_set);
    FD_SET(server_socket, &active_fd_set);
    max_fd = server_socket;

    // printf("Servidor %s iniciado na porta %d (%s)\n", server_type == 1 ? "SE" : "SCII", port, argv[1]);

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
                    struct sockaddr_storage client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (client_socket >= 0) {
                        if (num_clients >= MAX_CLIENTS) {
                            char error_msg[] = "ERROR(01)";
                            send(client_socket, error_msg, strlen(error_msg), 0);
                            close(client_socket);
                        } else {
                            FD_SET(client_socket, &active_fd_set);
                            if (client_socket > max_fd) {
                                max_fd = client_socket;
                            }
                            num_clients++;

                            // Atribui um ID único ao cliente e envia a mensagem de resposta
                            int client_id = next_client_id++;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (!clients[j].active) {
                                    clients[j].socket = client_socket;
                                    clients[j].id = client_id;
                                    clients[j].active = 1;
                                    break;
                                }
                            }
                            printf("Cliente %d added\n", client_id);
                            char response[BUFFER_SIZE];
                            snprintf(response, BUFFER_SIZE, "RES_ADD(%d)", client_id);
                            send(client_socket, response, strlen(response), 0);
                        }
                    }
                } else {
                    // Dados de um cliente existente
                    handle_client(i, server_type, &active_fd_set, &num_clients);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket, int server_type, fd_set *active_fd_set, int *num_clients) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE); // Limpa o buffer antes de receber dados
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == client_socket) {
                    // printf("Cliente %d desconectado.\n", clients[i].id);
                    clients[i].active = 0;
                    break;
                }
            }
        } else {
            perror("recv");
        }
        close(client_socket);
        FD_CLR(client_socket, active_fd_set);
        (*num_clients)--;
        return;
    }

    int client_id = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket) {
            client_id = clients[i].id;
            break;
        }
    }
    // printf("Recebido do cliente %d: %s\n", client_id, buffer);

    // Lógica específica para cada tipo de servidor
    switch (server_type) {
        case 1: // SE
            if (strncmp(buffer, "REQ_INFOSE", 10) == 0) {
                // Simula a produção de energia
                snprintf(buffer, BUFFER_SIZE, "RES_INFOSE(%d)", energy_production_history[energy_production_index-1]);
                send(client_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "REQ_REM(", 8) == 0) {
                int rem_client_id;
                sscanf(buffer, "REQ_REM(%d)", &rem_client_id);
                int found = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].id == rem_client_id && clients[i].active) {
                        found = 1;
                        FD_CLR(clients[i].socket, active_fd_set);
                        clients[i].active = 0;
                        (*num_clients)--;
                        printf("Cliente %d removed\n", rem_client_id);
                        char response[BUFFER_SIZE];
                        snprintf(response, BUFFER_SIZE, "OK(01)");
                        // printf("Send SE: %s\n", response);
                        send(client_socket, response, strlen(response), 0);
                        return;
                    }
                }
                if (!found) {
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "ERROR(02)");
                    send(client_socket, response, strlen(response), 0);
                }
            } else if (strncmp(buffer, "REQ_STATUS", 10) == 0) {
                    int energy_production = energy_production_history[energy_production_index-1];
                    generate_random_energy_production();
                    char state[BUFFER_SIZE];
                    if (energy_production >= 41) {
                        snprintf(state, BUFFER_SIZE, "alta");
                    } else if (energy_production >= 31) {
                        snprintf(state, BUFFER_SIZE, "moderada");
                    } else {
                        snprintf(state, BUFFER_SIZE, "baixa");
                    }
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "RES_STATUS(%s)", state);
                    send(client_socket, response, strlen(response), 0);
            } else {
                printf("Send SE: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            break;
        case 2: // SCII
            if (strncmp(buffer, "REQ_INFOSCII", 11) == 0) {
                // Simula o consumo de energia
                snprintf(buffer, BUFFER_SIZE, "RES_INFOSCII(%d)", consumption_history[consumption_index-1]);
                send(client_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "REQ_REM(", 8) == 0) {
                int rem_client_id;
                sscanf(buffer, "REQ_REM(%d)", &rem_client_id);
                int found = 0;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].id == rem_client_id && clients[i].active) {
                        found = 1;
                        FD_CLR(clients[i].socket, active_fd_set);
                        clients[i].active = 0;
                        (*num_clients)--;
                        printf("Cliente %d removed\n", rem_client_id);
                        char response[BUFFER_SIZE];
                        snprintf(response, BUFFER_SIZE, "OK(01)");
                        // printf("Send SCII: %s\n", response);
                        send(client_socket, response, strlen(response), 0);
                        return;
                    }
                }
                if (!found) {
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "ERROR(02)");
                    send(client_socket, response, strlen(response), 0);
                }
            } else if (strncmp(buffer, "REQ_UP", 6) == 0) {
                    handle_req_up(client_socket);
                } else if (strncmp(buffer, "REQ_NONE", 8) == 0) {
                    handle_req_none(client_socket);
                } else if (strncmp(buffer, "REQ_DOWN", 8) == 0) {
                    handle_req_down(client_socket);
            } else {
                printf("Send SCII: %s\n", buffer);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            break;
    }
    memset(buffer, 0, BUFFER_SIZE); // Limpa o buffer após o envio dos dados
}

void add_energy_production(int value) {
    energy_production_history[energy_production_index] = value;
    energy_production_index++;
}

void add_consumption(int value) {
    consumption_history[consumption_index] = value;
    consumption_index++;

}

void generate_random_energy_production() {
    int value = rand() % 31 + 20; 
    add_energy_production(value);
}

void generate_random_consumption() {
    int value = rand() % 101; 
    add_consumption(value);
}

void handle_req_up(int client_socket) {
    int old_value = consumption_history[consumption_index-1]; // Buscar valor antigo na base de dados
    int new_value = rand() % (100 - old_value + 1) + old_value; // Gera um valor maior ou igual ao antigo
    
    add_consumption(new_value); // Adiciona o novo valor ao histórico
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "RES_UP(%d,%d)", old_value, new_value);
    send(client_socket, response, strlen(response), 0);
}

void handle_req_none(int client_socket) {
    int current_value = consumption_history[consumption_index-1]; // Buscar valor atual na base de dados

    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "RES_NONE(%d)", current_value);
    send(client_socket, response, strlen(response), 0);
}

void handle_req_down(int client_socket) {
    int old_value = consumption_history[consumption_index-1]; // Buscar valor antigo na base de dados
    int new_value = old_value - (rand() % (old_value + 1)); // Gera um valor menor ou igual ao antigo
    
    add_consumption(new_value); // Adiciona o novo valor ao histórico

    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "RES_DOWN(%d,%d)", old_value, new_value);
    send(client_socket, response, strlen(response), 0);
}