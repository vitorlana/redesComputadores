#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#define BUFFER_SIZE 500

int se_client_id = -1;
int scii_client_id = -1;

void communicate_with_servers(int se_socket, int scii_socket);
void removeNewline(char *str);
void send_req_status(int se_socket);
void handle_res_status(char *response, int scii_socket);
void handle_res_up(char *response);
void handle_res_none(char *response);
void handle_res_down(char *response);

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

void removeNewline(char *str) {
    char *src, *dst;
    src = dst = str;
    
    while (*src) {
        if (*src != '\n') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

void communicate_with_servers(int se_socket, int scii_socket) {
    char bufferenv[BUFFER_SIZE];
    char buffersce[BUFFER_SIZE];
    char bufferscii[BUFFER_SIZE];

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
            memset(bufferenv, 0, BUFFER_SIZE); // Limpa o buffer antes de ler a entrada do usuário

            if (fgets(bufferenv, BUFFER_SIZE, stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }else if (strncmp(bufferenv, "kill", 4) == 0) {
                if (se_client_id != -1) {
                    snprintf(bufferenv, BUFFER_SIZE, "REQ_REM(%d)", se_client_id);
                    send(se_socket, bufferenv, strlen(bufferenv), 0);
                }
                if (scii_client_id != -1) {
                    snprintf(bufferenv, BUFFER_SIZE, "REQ_REM(%d)", scii_client_id);
                    send(scii_socket, bufferenv, strlen(bufferenv), 0);
                }
            }else if (strncmp(bufferenv, "display info se", 15) == 0) {
                    snprintf(bufferenv, BUFFER_SIZE, "REQ_INFOSE");
                    send(se_socket, bufferenv, strlen(bufferenv), 0);
            }else if (strncmp(bufferenv, "display info scii", 17) == 0) {
                    snprintf(bufferenv, BUFFER_SIZE, "REQ_INFOSCII");
                    send(scii_socket, bufferenv, strlen(bufferenv), 0);
            }else if (strncmp(bufferenv, "query condition", 15) == 0) {
                    snprintf(bufferenv, BUFFER_SIZE, "REQ_STATUS");
                    send(se_socket, bufferenv, strlen(bufferenv), 0);
            }else{
                printf("Comando inválido: %s\n", bufferenv);
            }
            memset(bufferenv, 0, BUFFER_SIZE); // Limpa o buffer após enviar a mensagem
        }

        if (FD_ISSET(se_socket, &read_fds)) {
            // Recebe a mensagem do servidor SE
            memset(buffersce, 0, BUFFER_SIZE); // Limpa o buffer antes de receber a mensagem
            int bytes_received = recv(se_socket, buffersce, BUFFER_SIZE, 0);
            // printf("From SCE: %s\n", buffersce);


            if (bytes_received > 0) {
                buffersce[bytes_received] = '\0';
                if (strncmp(buffersce, "ERROR(01)",9) == 0) {
                    printf("Client limit exceeded\n");
                    close(se_socket);
                    close(scii_socket);
                    exit(EXIT_FAILURE);
                } else if (strncmp(buffersce, "RES_ADD(", 8) == 0) {
                    sscanf(buffersce, "RES_ADD(%d)", &se_client_id);
                    printf("Servidor SE New ID: %d\n", se_client_id);
                } else if (strncmp(buffersce, "OK(01)", 6) == 0) {
                    printf("Successful disconnect\n");
                    close(se_socket);
                    se_client_id = -1;
                    if (scii_client_id == -1) {
                        exit(EXIT_SUCCESS);
                    }
                } else if (strncmp(buffersce, "ERROR(02)", 9) == 0) {
                    printf("Client not found\n");
                } else if (strncmp(buffersce, "RES_INFOSE(", 11) == 0) {
                    int prod_energy;
                    sscanf(buffersce, "RES_INFOSE(%d)", &prod_energy);
                    printf("producao atual: %d kWh\n", prod_energy);
                } else if (strncmp(buffersce, "RES_STATUS", 10) == 0) {
                    handle_res_status(buffersce, scii_socket);
                } else {
                    // printf("From SE: %s\n", buffer);
                }
            }
            memset(buffersce, 0, BUFFER_SIZE); // Limpa o buffer após processar a mensagem recebida
        }

        if (FD_ISSET(scii_socket, &read_fds)) {
            // Recebe a mensagem do servidor SCII
            memset(bufferscii, 0, BUFFER_SIZE); // Limpa o buffer antes de receber a mensagem

            int bytes_received = recv(scii_socket, bufferscii, BUFFER_SIZE, 0);
            // printf("From SCII: %s\n", bufferscii);

            if (bytes_received > 0) {
                bufferscii[bytes_received] = '\0';
                if (strncmp(bufferscii, "ERROR(01)", 9) == 0) {
                    printf("Client limit exceeded\n");
                    close(se_socket);
                    close(scii_socket);
                    exit(EXIT_FAILURE);
                } else if (strncmp(bufferscii, "RES_ADD(", 8) == 0) {
                    sscanf(bufferscii, "RES_ADD(%d)", &scii_client_id);
                    printf("Servidor SCII New ID: %d\n", scii_client_id);
                } else if (strncmp(bufferscii, "OK(01)", 6) == 0) {
                    printf("Successful disconnect\n");
                    close(scii_socket);
                    scii_client_id = -1;
                    if (se_client_id == -1) {
                        exit(EXIT_SUCCESS);
                    }
                } else if (strncmp(bufferscii, "ERROR(02)", 9) == 0) {
                    printf("Client not found\n");
                }else if (strncmp(bufferscii, "RES_INFOSCII(", 12) == 0) {
                    int cons_energy;
                    sscanf(bufferscii, "RES_INFOSCII(%d)", &cons_energy);
                    printf("consumo atual: %d %%\n", cons_energy);
                } else if (strncmp(bufferscii, "RES_UP", 6) == 0) {
                    handle_res_up(bufferscii);
                } else if (strncmp(bufferscii, "RES_NONE", 8) == 0) {
                    handle_res_none(bufferscii);
                } else if (strncmp(bufferscii, "RES_DOWN", 8) == 0) {
                    handle_res_down(bufferscii);
                } else {
                    // printf("From SCII: %s\n", bufferscii);
                }
            }
            memset(bufferscii, 0, BUFFER_SIZE); // Limpa o buffer após processar a mensagem recebida
        }
    }
}

void send_req_status(int se_socket) {
    char message[] = "REQ_STATUS";
    send(se_socket, message, strlen(message), 0);
}

void handle_res_status(char *response, int scii_socket) {
    char estado[BUFFER_SIZE];
    sscanf(response, "RES_STATUS(%[^)])", estado);
    printf("estado atual: %s\n", estado);

    if (strcmp(estado, "alta") == 0) {
        char message[] = "REQ_UP";
        send(scii_socket, message, strlen(message), 0);
    } else if (strcmp(estado, "moderada") == 0) {
        char message[] = "REQ_NONE";
        send(scii_socket, message, strlen(message), 0);
    } else if (strcmp(estado, "baixa") == 0) {
        char message[] = "REQ_DOWN";
        send(scii_socket, message, strlen(message), 0);
    }
}

void handle_res_up(char *response) {
    int old_value, new_value;
    sscanf(response, "RES_UP(%d,%d)", &old_value, &new_value);
    printf("consumo antigo: %d %%\n", old_value);
    printf("consumo atual: %d %%\n", new_value);
}

void handle_res_none(char *response) {
    int current_value;
    sscanf(response, "RES_NONE(%d)", &current_value);
    printf("consumo atual: %d %%\n", current_value);
}

void handle_res_down(char *response) {
    int old_value, new_value;
    sscanf(response, "RES_DOWN(%d,%d)", &old_value, &new_value);
    printf("consumo antigo: %d %%\n", old_value);
    printf("consumo atual: %d %%\n", new_value);
}
